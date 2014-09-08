#!/bin/bash -l

echo "In observeCalibrator.sh : this creates 1934 MS from pointings ${firstPointingID} to ${lastPointingID}"

spw="[${nchan}, ${freqChanZeroMHz} MHz, ${chanw} Hz, \"${pol}\"]"

echo "Creating a random gains parset for ${nfeeds} feeds, ${nant} antennas and ${nstokes} polarisations"
if [ $doCorrupt == true ]; then
    $rndgains -f ${nfeeds} -a ${nant} -p ${nstokes} $randomgainsparset
fi

POINTING=${firstPointingID}
MAXPOINTING=${lastPointingID}
calMSlist=""
mergeCalDep="-d afterok"
while [ $POINTING -le $MAXPOINTING ]; do

    beamName=`echo $POINTING | awk '{printf "BEAM%02d",$1}'`

    . ${simScripts}/makeFeedParset.sh

    ms=${msdir}/${msbaseCal}_${beamName}.ms
    calMSlist="$calMSlist $ms"

    sbatchfile=csimCalibrator_${beamName}.sbatch
    cat > $sbatchfile <<EOF
#!/bin/bash -l
#SBATCH --time=06:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name csimCal${POINTING}
#SBATCH --mail-user=matthew.whiting@csiro.au
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

csim=${csim}

rm -rf $ms

mkVisParset=${parsetdir}/csimCalibrator-${beamName}-\${SLURM_JOB_ID}.in
mkVisLog=${logdir}/csimCalibrator-${beamName}-\${SLURM_JOB_ID}.log

cat > \${mkVisParset} << EOF_INNER
Csimulator.dataset                              =       ${ms}
#
Csimulator.stman.bucketsize                     =       2097152
#
Csimulator.sources.names                        =       [DCmodel]
Csimulator.sources.DCmodel.direction            =       ${direction1934}
Csimulator.sources.DCmodel.components            =       src
Csimulator.sources.src.calibrator                =       "1934-638"
#
# Define the antenna locations, feed locations, and spectral window definitions
#
Csimulator.antennas.definition                   =       ${antennaParset}
Csimulator.feeds.definition                      =       ${feedparset}
Csimulator.spws.names                            =       [thisSPWS]
Csimulator.spws.thisSPWS                         =       ${spw}
#						 
Csimulator.simulation.blockage                   =       0.01
Csimulator.simulation.elevationlimit             =       8deg
Csimulator.simulation.autocorrwt                 =       0.0
Csimulator.simulation.usehourangles              =       True
Csimulator.simulation.referencetime              =       [2010Jan30, UTC]
#						 
Csimulator.simulation.integrationtime            =       5s
#						 
Csimulator.observe.number                        =       1
Csimulator.observe.scan0                         =       [DCmodel, thisSPWS, ${calHArange[$POINTING]}]
#
Csimulator.gridder                               =       AWProject
Csimulator.gridder.padding                       =       1.
Csimulator.gridder.snapshotimaging               =       false
Csimulator.gridder.snapshotimaging.wtolerance    =       1000
Csimulator.gridder.AWProject.wmax               =       1000
Csimulator.gridder.AWProject.nwplanes           =       ${nw}
Csimulator.gridder.AWProject.oversample         =       ${os}
Csimulator.gridder.AWProject.diameter           =       12m
Csimulator.gridder.AWProject.blockage           =       2m
Csimulator.gridder.AWProject.maxsupport         =       512
Csimulator.gridder.AWProject.maxfeeds           =       ${nfeeds}
Csimulator.gridder.AWProject.frequencydependent =       false
Csimulator.gridder.AWProject.variablesupport    =       true 
Csimulator.gridder.AWProject.offsetsupport      =       true 
#
Csimulator.noise                                 =       true
Csimulator.noise.Tsys                            =       50.
Csimulator.noise.efficiency                      =       0.8   
Csimulator.noise.seed1                           =       time
Csimulator.noise.seed2                           =       0
#
Csimulator.corrupt                               =       ${doCorrupt}
Csimulator.calibaccess                           =       parset
Csimulator.calibaccess.parset                    =       $randomgainsparset
EOF_INNER

aprun \${csim} -c \${mkVisParset} > \${mkVisLog}

EOF

    if [ $doSubmit == true ]; then
	latestID=`sbatch ${sbatchfile} | awk '{print $4}'`
	echo "Running csimulator for pointing ${POINTING} with 1934-638 component, producing measurement set ${ms}: ID=${latestID}"
	mergeCalDep="${mergeCalDep}:${latestID}"
    fi

    POINTING=`expr $POINTING + 1`

done

##################
# Merge all calibrator MSs into one.

mergeCalsbatch=mergeCalVis.sbatch
	
cat > $mergeCalsbatch <<EOF
#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=12:00:00
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name visMergeCal
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

ulimit -n 8192
export APRUN_XFER_LIMITS=1

msmerge=${msmerge}

IDX=0

FILES="${calMSlist}"

logfile=${logdir}/merge_cal_output_\${SLURM_JOB_ID}.log
echo "Processing files: " > \${logfile}
aprun -n 1 -N 1 \${msmerge} -o ${msdir}/${msbaseCal}.ms \$FILES >> \${logfile}
EOF

if [ $doSubmit == true ]; then
	
    merge2ID=`sbatch ${mergeCalDep} $mergeCalsbatch | awk '{print $4}'`
    echo "Running merging for full science field: ID=${merge2ID} and dependency $merge2dep"

fi





