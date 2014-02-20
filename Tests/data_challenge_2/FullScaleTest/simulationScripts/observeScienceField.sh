#!/bin/bash -l

merge2Dep="-Wdepend=afterok"
GRP=0
while [ $GRP -lt ${NGROUPS_CSIM} ]; do

    echo "Running group ${GRP} of ${NGROUPS_CSIM}"

    ms=${msdir}/${msbaseSci}_GRP${GRP}_%w.ms

    depend=""
    if [ $doFlatSpectrum == true ]; then 
	skymodel=${slicedir}/${baseimage}
    else
	slicebase="${slicedir}/${baseimage}_GRP${GRP}_slice"
	skymodel=${slicebase}%w
	if [ $doSlice == true ]; then 
	    firstChanSlicer=`echo $GRP $NWORKERS_CSIM $chanPerMSchunk | awk '{print $1*$2*$3}'`
	    nchanSlicer=`echo $NWORKERS_CSIM $chanPerMSchunk | awk '{print $1*$2}'`
	    . ${simScripts}/makeSlices.sh
	    depend="-Wdepend=afterok:${slID}"
	    merge2dep="${merge2dep}:${slID}"
	fi
    fi

    # Need to create an spws file for this group with the
    # appropriate channel settings, so we can reference with %w in
    # the parset
    spwsInput="${spwbaseSci}_grp${GRP}.in"
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

    mkVisParset=${parsetdir}/csim-Science-GRP${GRP}.in
    mkVisLog=${logdir}/csim-Science-GRP${GRP}.log

    cat > ${mkVisParset} << EOF_INNER
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
Csimulator.antennas.definition                   =       ${askapconfig}/BETAXYZ.in
Csimulator.feeds.definition                      =       ${askapconfig}/ASKAP9feeds.in
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
Csimulator.gridder.AWProject.maxsupport          =       512
Csimulator.gridder.AWProject.maxfeeds            =       9
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

    pbstag="csimSci${GRP}"
    qsubfile=csim_Science_${tag}.qsub
    cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N ${pbstag}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

rm -rf $ms

aprun ${csim} -c ${mkVisParset} > ${mkVisLog}

EOF

    if [ $doSubmit == true ]; then
	csimID=`qsub ${depend} ${qsubfile}`
	if [ "$depend" == "" ]; then
	    depend="-Wdepend=afterok:${csimID}"
	else
	    depend="${depend}:${csimID}"
	fi
	merge2dep="${merge2Dep}:${csimID}"
    fi
    fi

    echo "Running csimulator for science field, producing measurement set ${ms}: ID=${csimID}"

    merge1qsub=${parsetdir}/mergeVisStage1_GRP${GRP}.qsub
    
    cat > $merge1qsub <<EOF
#!/bin/bash
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -l walltime=12:00:00
#PBS -M matthew.whiting@csiro.au
#PBS -N visMerge1_${GRP}
#PBS -m a
#PBS -j oe

#######
# AUTOMATICALLY CREATED
#######

ulimit -n 8192
export APRUN_XFER_LIMITS=1

cd \$PBS_O_WORKDIR

MSPERJOB=${NWORKERS_CSIM}

IDX=0
unset FILES
while [ \$IDX -lt \$MSPERJOB ]; do
    FILES="\$FILES ${msdir}/${msbaseSci}_GRP${GRP}_\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

mkdir -p ${logdir}
logfile=${logdir}/merge_s1_output_GRP${GRP}_\${PBS_JOBID}.log
echo "Start = \$START, End = \$END" > \${logfile}
echo "Processing files: \$FILES" >> \${logfile}
aprun $ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh -o ${msdir}/${msbaseSci}_GRP${GRP}.ms \$FILES >> \${logfile}

EOF

    if [ $doSubmit == true ]; then
	merge1ID=`qsub ${depend} $merge1qsub`
	merge2dep="${merge2dep}:${merge1ID}"
    fi

    GRP=`expr $GRP + 1`
    
done



    if [ $doMergeStage2 == true ]; then

	merge2qsub=${visdir}/${WORKDIR}/mergeVisStage2.qsub
	
	cat > $merge2qsub <<EOF
#!/bin/bash
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -l walltime=12:00:00
#PBS -M matthew.whiting@csiro.au
#PBS -N visMerge2
#PBS -m a
#PBS -j oe

ulimit -n 8192
export APRUN_XFER_LIMITS=1

cd \$PBS_O_WORKDIR

IDX=0
unset FILES
while [ \$IDX -lt ${NGROUPS_CSIM} ]; do
    FILES="\$FILES ${msdir}/${msbaseSci}_GRP\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

logfile=${logdirVis}/${WORKDIR}/merge_s2_output_\${PBS_JOBID}.log
echo "Processing files: \$FILES" > \${logfile}
aprun $ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh -o ${msdir}/${msbaseSci}.ms \$FILES >> \${logfile}
EOF

	if [ $doSubmit == true ]; then

	    merge2ID=`qsub ${merge2dep} $merge2qsub`

	fi

    fi

fi

