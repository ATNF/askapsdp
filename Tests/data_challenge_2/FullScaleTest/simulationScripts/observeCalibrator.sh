#!/bin/bash -l


spw="[${nchan}, ${freqChanZeroMHz} MHz, ${chanw} Hz, \"${pol}\"]"

POINTING=${firstPointingID}
MAXPOINTING=${lastPointingID}
while [ $POINTING -le $MAXPOINTING ]; do

    . ${simScripts}/makeFeedParset.sh

    mkVisParset=${parsetdir}/csimCalibrator-BEAM${POINTING}.in
    mkVisLog=${logdir}/csimCalibrator-BEAM${POINTING}.log

    ms=${msdir}/${msbaseCal}_BEAM${POINTING}.ms

    cat > ${mkVisParset} << EOF_INNER
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
Csimulator.antennas.definition                   =       ${askapconfig}/BETAXYZ.in
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
Csimulator.gridder.AWProject.maxfeeds           =       9
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

    sbatchfile=csimCalibrator_BEAM${POINTING}.sbatch
    cat > $sbatchfile <<EOF
#!/bin/bash -l
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name csimCal${POINTING}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

csim=${csim}

rm -rf $ms

aprun \${csim} -c ${mkVisParset} > ${mkVisLog}

EOF

    if [ $doSubmit == true ]; then
	latestID=`sbatch ${sbatchfile}`
	echo "Running csimulator for pointing ${POINTING} with 1934-638 component, producing measurement set ${ms}: ID=${latestID}"
    fi

    POINTING=`expr $POINTING + 1`

done





