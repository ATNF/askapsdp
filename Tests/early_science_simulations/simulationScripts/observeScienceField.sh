#!/bin/bash -l

echo "In observeScienceField.sh : this creates ${NGROUPS_CSIM} groups of science-field MSs, that are then merged in two stages"

if [ "$depend" == "" ]; then
    merge2dep="-d afterok"
else
    merge2dep=${depend}
fi

GRP=0
while [ $GRP -lt ${NGROUPS_CSIM} ]; do

    echo "Running group ${GRP} of ${NGROUPS_CSIM}"

    ms=${msdir}/${msbaseSci}_GRP${GRP}_%w.ms

    grpDepend=$depend
    if [ $doFlatSpectrum == true ]; then 
	skymodel=${slicedir}/${baseimage}
    else
	slicebase="${slicedir}/${baseimage}_GRP${GRP}_slice"
	skymodel=${slicebase}%w
	if [ $doSlice == true ]; then 
	    firstChanSlicer=`echo $GRP $NWORKERS_CSIM $chanPerMSchunk | awk '{print $1*$2*$3}'`
	    nchanSlicer=`echo $NWORKERS_CSIM $chanPerMSchunk | awk '{print $1*$2}'`
	    . ${simScripts}/makeSlices.sh
	    if [ "$grpDepend" == "" ]; then
		grpDepend="-d afterok:${slID}"
	    else
		grpDepend="${grpDepend}:${slID}"
	    fi
	    merge2dep="${merge2dep}:${slID}"
	fi
    fi

    # Need to create an spws file for this group with the
    # appropriate channel settings, so we can reference with %w in
    # the parset
    spwsInput="${spwbaseSci}_GRP${GRP}.in"
    echo "spws.names  =  [GRP${GRP}_0" > $spwsInput
    I=1; while [ $I -lt ${NWORKERS_CSIM} ]; do echo ", GRP${GRP}_${I}" >> $spwsInput; I=`expr $I + 1`; done
    perl -pi -e 's/\n//g' $spwsInput
    echo "]" >> $spwsInput
    I=0
    while [ $I -lt ${NWORKERS_CSIM} ]; do 
	nurefMHz=`echo ${freqChanZeroMHz} ${GRP} ${NWORKERS_CSIM} ${I} ${chanPerMSchunk} ${rchan} ${chanw} | awk '{printf "%13.8f",($1*1.e6+(($2*$3+$4)*$5-$6)*$7)/1.e6}'`
	spw="[${chanPerMSchunk}, ${nurefMHz} MHz, ${chanw} Hz, \"${pol}\"]"
	echo "spws.GRP${GRP}_${I}  =  ${spw}" >> $spwsInput
	I=`expr $I + 1`
    done

    slurmtag="csimSci${GRP}"
    sbatchfile=csimScience_GRP${GRP}.sbatch
    cat > $sbatchfile <<EOF
#!/bin/bash -l
#SBATCH --time=06:00:00
#SBATCH --ntasks=${NCPU_CSIM}
#SBATCH --ntasks-per-node=${NPPN_CSIM}
#SBATCH --job-name ${slurmtag}
#SBATCH --mail-user=matthew.whiting@csiro.au
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

csim=${csim}

rm -rf $ms

mkVisParset=${parsetdir}/csimScience-GRP${GRP}.in
mkVisLog=${logdir}/csimScience-GRP${GRP}.log

cat > \${mkVisParset} << EOF_INNER
Csimulator.dataset                              =       ${ms}
#
Csimulator.stman.bucketsize                     =       2097152
#
Csimulator.sources.names                        =       [DCmodel]
Csimulator.sources.DCmodel.direction            =       ${baseDirection}
Csimulator.sources.DCmodel.model                =       ${skymodel}
Csimulator.modelReadByMaster                    =       false
#
# Define the antenna locations, feed locations, and spectral window definitions
#
Csimulator.antennas.definition                   =       ${antennaParset}
Csimulator.feeds.definition                      =       ${feeds}
Csimulator.spws.definition                       =       ${spwsInput}
#						 
Csimulator.simulation.blockage                   =       0.01
Csimulator.simulation.elevationlimit             =       8deg
Csimulator.simulation.autocorrwt                 =       0.0
Csimulator.simulation.usehourangles              =       True
Csimulator.simulation.referencetime              =       [2010Jan30, UTC]
#						 
Csimulator.simulation.integrationtime            =       ${inttime}
#						 
Csimulator.observe.number                        =       1
Csimulator.observe.scan0                         =       [DCmodel, GRP${GRP}_%w, -${dur}h, ${dur}h]
#
Csimulator.gridder                               =       AWProject
Csimulator.gridder.padding                       =       1.
Csimulator.gridder.snapshotimaging               =       true
Csimulator.gridder.snapshotimaging.wtolerance    =       1000
Csimulator.gridder.AWProject.wmax                =       1000
Csimulator.gridder.AWProject.nwplanes            =       ${nw}
Csimulator.gridder.AWProject.oversample          =       ${os}
Csimulator.gridder.AWProject.diameter            =       12m
Csimulator.gridder.AWProject.blockage            =       2m
Csimulator.gridder.AWProject.maxsupport          =       1024
Csimulator.gridder.AWProject.maxfeeds            =       ${nfeeds}
Csimulator.gridder.AWProject.frequencydependent  =       false
Csimulator.gridder.AWProject.variablesupport     =       true 
Csimulator.gridder.AWProject.offsetsupport       =       true 
#
Csimulator.noise                                 =       ${doNoise}
Csimulator.noise.Tsys                            =       ${Tsys}
Csimulator.noise.efficiency                      =       0.8   
Csimulator.noise.seed1                           =       time
Csimulator.noise.seed2                           =       %w
#
Csimulator.corrupt                               =       ${doCorrupt}
Csimulator.calibaccess                           =       parset
Csimulator.calibaccess.parset                    =       $randomgainsparset
EOF_INNER

aprun -n ${NCPU_CSIM} -N ${NPPN_CSIM}  \${csim} -c \${mkVisParset} > \${mkVisLog}

EOF

    if [ $doSubmit == true ]; then
	csimID=`sbatch ${grpDepend} ${sbatchfile} | awk '{print $4}'`
	echo "Running csimulator for science field, group $GRP, producing measurement set ${ms}: ID=${csimID} and dependency $grpDepend"
	if [ "$depend" == "" ]; then
	    grpDepend="-d afterok:${csimID}"
	else
	    grpDepend="${grpDepend}:${csimID}"
	fi
	merge2dep="${merge2dep}:${csimID}"
    fi


    merge1sbatch=mergeVisStage1_GRP${GRP}.sbatch
    
    cat > $merge1sbatch <<EOF
#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=12:00:00
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name visMerge1_${GRP}
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

#######
# AUTOMATICALLY CREATED
#######

ulimit -n 8192
export APRUN_XFER_LIMITS=1

msmerge=${msmerge}

MSPERJOB=${NWORKERS_CSIM}

IDX=0
unset FILES
while [ \$IDX -lt \$MSPERJOB ]; do
    FILES="\$FILES ${msdir}/${msbaseSci}_GRP${GRP}_\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

mkdir -p ${logdir}
logfile=${logdir}/merge_s1_output_GRP${GRP}_\${SLURM_JOB_ID}.log
echo "Start = \$START, End = \$END" > \${logfile}
echo "Processing files: \$FILES" >> \${logfile}
aprun -n 1 -N 1 \${msmerge} -o ${msdir}/${msbaseSci}_GRP${GRP}.ms \$FILES >> \${logfile}

EOF

    if [ $doSubmit == true ]; then
	merge1ID=`sbatch ${grpDepend} $merge1sbatch | awk '{print $4}'`
	echo "Running merging for science field, group ${GRP}: ID=${merge1ID} and dependency $grpDepend"
	merge2dep="${merge2dep}:${merge1ID}"
    fi

    GRP=`expr $GRP + 1`
    
done



merge2sbatch=mergeVisStage2.sbatch
	
cat > $merge2sbatch <<EOF
#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=12:00:00
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name visMerge2
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

ulimit -n 8192
export APRUN_XFER_LIMITS=1

msmerge=${msmerge}

IDX=0
unset FILES
while [ \$IDX -lt ${NGROUPS_CSIM} ]; do
    FILES="\$FILES ${msdir}/${msbaseSci}_GRP\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

logfile=${logdir}/merge_s2_output_\${SLURM_JOB_ID}.log
echo "Processing files: \$FILES" > \${logfile}
aprun -n 1 -N 1 \${msmerge} -o ${msdir}/${msbaseSci}.ms \$FILES >> \${logfile}
EOF

if [ $doSubmit == true ]; then
	
    merge2ID=`sbatch ${merge2dep} $merge2sbatch | awk '{print $4}'`
    echo "Running merging for full science field: ID=${merge2ID} and dependency $merge2dep"

fi

