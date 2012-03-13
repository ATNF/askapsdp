#!/bin/bash -l

mkdir -p ${msdir}
mkdir -p ${parsetdirVis}
mkdir -p ${logdirVis}

cd ${visdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}

##############################
# Parameters & Definitions
##############################

dependVis=""
runOK=true

if [ $doCsim == true ]; then

    if [ $doVisCleanup == true ]; then
	
	if [ -e ${failureListVis} ]; then
	    INDEX="\`head -\${PBS_ARRAY_INDEX} ${failureListVis} | tail -1\`"
	    qsubCmd="qsub -J 1-\`wc -l ${failureListVis} | awk '{print $1}'\` "
	else
	    echo "Visibility failure list ${failureListVis} does not exist. Not running"
	    runOK=false
	fi
	
    else

	INDEX="\${PBS_ARRAY_INDEX}"
	qsubCmd="qsub -J 0-$maxChunkMS "
	
    fi 


##############################
# Qsub & Parset definition
##############################

    qsubfile=${visdir}/${WORKDIR}/makeVis-${now}.qsub

    cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=2:00:00
#PBS -l select=1:ncpus=1:mem=20GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N mkVis
#PBS -m bea
#PBS -j oe

cd \$PBS_O_WORKDIR
export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
csim=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/install/bin/csimulator.sh
askapconfig=\${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

IND=${INDEX}

ms=${msbase}_\${IND}.ms
skymodel=${slicebase}\${IND}
nuref=`echo \${IND} ${numPerChunk} ${chanWidth_kHz} | awk '{printf "%13.8f",1421.-($1*$2-1)*$3/1000.}'`
spw="[${numPerChunk}, \${nuref} MHz, -${chanWidth_kHz} kHz, \"XX YY\"]"

mkVisParset=${parsetdirVis}/csim-\${PBS_JOBID}.in
mkVisLog=${logdirVis}/csim-\${PBS_JOBID}.log

cat > \${mkVisParset} $begin
Csimulator.dataset                              =       \$ms
#
Csimulator.stman.bucketsize                     =       2097152
#
Csimulator.sources.names                        =       [DCmodel]
Csimulator.sources.DCmodel.direction            =       [12h30m00.000, -45.00.00, J2000]
Csimulator.sources.DCmodel.model                =       \${skymodel}
#
# Define the antenna locations, feed locations, and spectral window definitions
#
Csimulator.antennas.definition                   =       \${askapconfig}/${array}
Csimulator.feeds.definition                      =       \${askapconfig}/${feeds}
Csimulator.spws.names                            =       [thisSPWS]
Csimulator.spws.thisSPWS                         =       \${spw}
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
Csimulator.observe.scan0                         =       [DCmodel, thisSPWS, -${dur}h, ${dur}h]
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
Csimulator.gridder.${gridder}.maxfeeds           =       36
Csimulator.gridder.${gridder}.frequencydependent =       false
Csimulator.gridder.${gridder}.variablesupport    =       true 
Csimulator.gridder.${gridder}.offsetsupport      =       true 
#
Csimulator.noise                                 =       ${doNoise}
Csimulator.noise.Tsys                            =       ${tsys}
Csimulator.noise.efficiency                      =       0.8   
$end

mpirun \${csim} -inputs \${mkVisParset} > \${mkVisLog}
err=\$?
exit \$err

EOF

    if [ $doSubmit == true ] && [ $runOK == true ]; then
	
	visID=`$qsubCmd ${depend} $qsubfile`
	dependVis="-W depend=afterok:${visID}"
	
    fi

fi


##########################
# Merging of visibilities

if [ $doMergeVis == true ]; then

    if [ $doMergeStage1 == true ]; then

	merge1qsub=${visdir}/${WORKDIR}/mergeVisStage1-${now}.qsub
    
	cat > $merge1qsub <<EOF
#!/bin/bash
#PBS -W group_list=astronomy116
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
#PBS -l walltime=12:00:00
#PBS -M matthew.whiting@csiro.au
#PBS -N visMerge1
#PBS -m a
#PBS -j oe

#######
# TO RUN (20 jobs):
#  qsub -J 0-19 stage1.qsub
#######

cd \$PBS_O_WORKDIR

MSPERJOB=41

START=\`expr \${PBS_ARRAY_INDEX} \* \$MSPERJOB\`
END=\`expr \${START} + \${MSPERJOB}\`

IDX=\$START
unset FILES
while [ \$IDX -lt \$END ]; do
    FILES="\$FILES ${msbase}_\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

logfile=${logdirVis}/merge_s1_output_\${PBS_ARRAY_INDEX}.log
echo "Processing files: $FILES" > \${logfile}
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh -o ${msStage1base}_\${PBS_ARRAY_INDEX}.ms \$FILES >> \${logfile}

EOF

	if [ $doSubmit == true ] && [ $runOK == true ]; then
	    
	    merge1ID=`qsub ${dependVis} -J 0-19 $merge1qsub`
	    
	    if [ "$dependVis" == "" ]; then
		dependVis="-W depend=afterok:${merge1ID}"
	    else
		dependVis="${dependVis}:${merge1ID}"
	    fi
	    
	fi

    fi

####

    if [ $doMergeStage2 == true ]; then

	merge2qsub=${visdir}/${WORKDIR}/mergeVisStage2-${now}.qsub

	cat > $merge2qsub <<EOF
#!/bin/bash
#PBS -W group_list=astronomy116
#PBS -l select=1:ncpus=1:mem=8GB:mpiprocs=1
#PBS -l walltime=12:00:00
#PBS -M matthew.whiting@csiro.au
#PBS -N visMerge2
#PBS -m a
#PBS -j oe

cd \$PBS_O_WORKDIR

IDX=0
unset FILES
while [ \$IDX -lt 20 ]; do
    FILES="\$FILES ${msStage1base}_\${IDX}.ms" 
    IDX=\`expr \$IDX + 1\`
done

logfile=${logdirVis}/merge_s2_output_\${PBS_JOBID}.log
echo "Processing files: \$FILES" > \${logfile}
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh -o ${finalMS} \$FILES >> \${logfile}
EOF

	if [ $doSubmit == true ] && [ $runOK == true ]; then

	    merge2ID=`qsub ${dependVis} $merge2qsub`

	fi

    fi

fi

cd ${BASEDIR}
