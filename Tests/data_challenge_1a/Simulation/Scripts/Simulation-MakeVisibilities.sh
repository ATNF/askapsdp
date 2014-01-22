#!/bin/bash -l

mkdir -p ${msdir}
mkdir -p ${parsetdirVis}
mkdir -p ${logdirVis}

cd ${visdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}
touch ${now}

##############################
# Parameters & Definitions
##############################

dependVis=""
runOK=true

if [ $doCsim == true ]; then

    if [ $doCorrupt == true ]; then
	# Create the random gains parset
	${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/randomgains.sh ${randomgainsArgs} ${calibparset}
    fi

    mergeDep="-Wdepend=afterok"
    GRP=0
    while [ $GRP -lt ${NGROUPS_CSIM} ]; do

	echo "Running group ${GRP} of ${NGROUPS_CSIM}"

	ms=${msSlice}_GRP${GRP}_%w.ms

	if [ $doFlatSpectrum == true ]; then 
	    skymodel=$slicebase
	else
	    slicebaseOrig=${slicebase}
	    sedString="s/_slice\$/_GRP${GRP}_slice/g"
	    slicebase=`echo ${slicebase} | sed -e $sedString`
	    skymodel=${slicebase}%w
	    firstChanSlicer=`echo $GRP $NWORKERS_CSIM $chanPerMSchunk | awk '{print $1*$2*$3}'`
	    nchanSlicer=`echo $NWORKERS_CSIM $chanPerMSchunk | awk '{print $1*$2}'`
	    . ${scriptdir}/Simulation-MakeSlices.sh
	    if [ $doSlice == true ]; then 
		mv ${visdir}/${WORKDIR}/makeslices.qsub ${visdir}/${WORKDIR}/makeslices_GRP${GRP}.qsub
		mergeDep="${mergeDep}:${slID}"
	    fi
	    slicebase=${slicebaseOrig}
	fi

	# Need to create an spws file for this group with the
	# appropriate channel settings, so we can reference with %w in
	# the parset
	spwsInput="spws_grp${GRP}.in"
	echo "spws.names  =  [GRP${GRP}_0" > $spwsInput
	I=1; while [ $I -lt ${NWORKERS_CSIM} ]; do echo ", GRP${GRP}_${I}" >> $spwsInput; I=`expr $I + 1`; done
	perl -pi -e 's/\n//g' $spwsInput
	echo "]" >> $spwsInput
	I=0
	while [ $I -lt ${NWORKERS_CSIM} ]; do 
	    nurefMHz=`echo ${rfreq} ${GRP} ${NWORKERS_CSIM} ${I} ${chanPerMSchunk} ${rchan} ${chanw} | awk '{printf "%13.8f",($1+(($2*$3+$4)*$5-$6)*$7)/1.e6}'`
	    spw="[${chanPerMSchunk}, ${nurefMHz} MHz, ${chanw} Hz, \"${pol}\"]"
	    echo "spws.GRP${GRP}_${I}  =  ${spw}" >> $spwsInput
	    I=`expr $I + 1`
	done

	VarNoise=${varNoise}
	Tsys=${tsys}
	if [ \${VarNoise} == true ]; then
	    Tsys=`echo $nurefMHz $noiseSlope $noiseIntercept $freqTsys50 | awk '{if ($1>$4) printf "%4.1f",($1 * $2) + $3; else printf "50.0"}'`
	fi

	qsubfile=makeVis_GRP${GRP}.qsub
	cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=6:00:00
#PBS -l mppwidth=${NCPU_CSIM}
#PBS -l mppnppn=${NPPN_CSIM}
#PBS -M matthew.whiting@csiro.au
#PBS -N mkVis${GRP}
#PBS -m bea
#PBS -j oe

#####
# AUTOMATICALLY CREATED!
#####

cd \$PBS_O_WORKDIR
export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
makeModelSlice=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/makeModelSlice.sh
csim=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/csimulator.sh
askapconfig=\${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

mkdir -p ${parsetdirVis}/${WORKDIR}
mkdir -p ${logdirVis}/${WORKDIR}
mkVisParset=${parsetdirVis}/${WORKDIR}/csim-\${PBS_JOBID}.in
mkVisLog=${logdirVis}/${WORKDIR}/csim-\${PBS_JOBID}.log

cat > \${mkVisParset} << EOF_INNER
Csimulator.dataset                              =       $ms
#
Csimulator.stman.bucketsize                     =       2097152
#
Csimulator.sources.names                        =       [DCmodel]
Csimulator.sources.DCmodel.direction            =       [12h30m00.000, ${decStringVis}, J2000]
Csimulator.sources.DCmodel.model                =       ${skymodel}
Csimulator.modelReadByMaster                    =       false
#
# Define the antenna locations, feed locations, and spectral window definitions
#
Csimulator.antennas.definition                   =       \${askapconfig}/${array}
Csimulator.feeds.definition                      =       \${askapconfig}/${feeds}
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
Csimulator.gridder                               =       ${gridder}
Csimulator.gridder.padding                       =       ${pad}
Csimulator.gridder.snapshotimaging               =       ${doSnapshot}
Csimulator.gridder.snapshotimaging.wtolerance    =       ${wtol}
Csimulator.gridder.${gridder}.wmax               =       ${wmax}
Csimulator.gridder.${gridder}.nwplanes           =       ${nw}
Csimulator.gridder.${gridder}.oversample         =       ${os}
Csimulator.gridder.${gridder}.diameter           =       12m
Csimulator.gridder.${gridder}.blockage           =       2m
Csimulator.gridder.${gridder}.maxsupport         =       ${maxsup}
Csimulator.gridder.${gridder}.maxfeeds           =       ${nfeeds}
Csimulator.gridder.${gridder}.frequencydependent =       ${doFreqDep}
Csimulator.gridder.${gridder}.variablesupport    =       true 
Csimulator.gridder.${gridder}.offsetsupport      =       true 
#
Csimulator.noise                                 =       ${doNoise}
Csimulator.noise.Tsys                            =       ${Tsys}
Csimulator.noise.efficiency                      =       0.8   
Csimulator.noise.seed1                           =       time
Csimulator.noise.seed2                           =       %w
#
Csimulator.corrupt                               =       ${doCorrupt}
Csimulator.calibaccess                           =       parset
Csimulator.calibaccess.parset                    =       ${calibparset}
EOF_INNER

aprun -n ${NCPU_CSIM} -N ${NPPN_CSIM} \${csim} -c \${mkVisParset} > \${mkVisLog}


EOF

	if [ $doSubmit == true ]; then
	    if [ $doSlice == true ]; then
		mkvisID=`qsub -Wdepend=afterok:${slID} ${qsubfile}`
	    else
		mkvisID=`qsub ${qsubfile}`
	    fi
	    mergeDep="${mergeDep}:${mkvisID}"
	fi

	
	if [ $doMergeStage1 == true ]; then

	    merge1qsub=${visdir}/${WORKDIR}/mergeVisStage1_GRP${GRP}.qsub
	    
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
    FILES="\$FILES ${msSlice}_GRP${GRP}_\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

mkdir -p ${logdirVis}/${WORKDIR}
logfile=${logdirVis}/${WORKDIR}/merge_s1_output_\${PBS_JOBID}.log
echo "Start = \$START, End = \$END" > \${logfile}
echo "Processing files: \$FILES" >> \${logfile}
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh -o ${msStage1}_${GRP}.ms \$FILES >> \${logfile}

EOF

	    if [ $doSubmit == true ]; then
		merge1ID=`qsub -Wdepend=afterok:${mkvisID} $merge1qsub`
		mergeDep="${mergeDep}:${merge1ID}"
	    fi

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
    FILES="\$FILES ${msStage1}_\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

logfile=${logdirVis}/${WORKDIR}/merge_s2_output_\${PBS_JOBID}.log
echo "Processing files: \$FILES" > \${logfile}
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh -o ${finalMS} \$FILES >> \${logfile}
EOF

	if [ $doSubmit == true ]; then

	    merge2ID=`qsub ${mergeDep} $merge2qsub`

	fi

    fi

fi

cd ${BASEDIR}
