#!/bin/bash -l


. ${scriptdir}/getTags.sh
. ${scriptdir}/makeFeedParset.sh

mkVisParset=parsets/csim-${tag}-${POINTING}.in
mkVisLog=logs/csim-${tag}-${POINTING}.log

thisModelImage=${modelImage}.${POINTING}

#direction=${dirlist[$POINTING]}
direction=${baseDirection}

ms=${msbase}_${mstag}_${POINTING}.ms

spw="[1, ${nurefMHz} MHz, ${chanw} Hz, \"${pol}\"]"

cat > ${mkVisParset} << EOF_INNER
Csimulator.dataset                              =       ${ms}
#
Csimulator.stman.bucketsize                     =       2097152
#
Csimulator.sources.names                        =       [DCmodel]
Csimulator.sources.DCmodel.direction            =       ${direction}
#Csimulator.sources.DCmodel.model                =       ${thisModelImage}
#Csimulator.modelReadByMaster                    =       false
Csimulator.sources.DCmodel.components            =       src
Csimulator.sources.src.calibrator                =       "1934-638"
#
# Define the antenna locations, feed locations, and spectral window definitions
#
Csimulator.antennas.definition                   =       ${askapconfig}/BETAXYZ.in
Csimulator.feeds.definition                      =       ${feedparset}
#Csimulator.spws.definition                       =       spws_grp0.in
Csimulator.spws.names                            =  [thisSPWS]
Csimulator.spws.thisSPWS                         = ${spw}
#						 
Csimulator.simulation.blockage                   =       0.01
Csimulator.simulation.elevationlimit             =       8deg
Csimulator.simulation.autocorrwt                 =       0.0
Csimulator.simulation.usehourangles              =       True
Csimulator.simulation.referencetime              =       [2010Jan30, UTC]
#						 
Csimulator.simulation.integrationtime            =       30s
#						 
Csimulator.observe.number                        =       1
#Csimulator.observe.scan0                         =       [DCmodel, thisSPWS, -6h, 6h]
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

qsubfile=csim_${POINTING}_${tag}.qsub
cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N csim${POINTING}${tag}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

rm -rf $ms

aprun ${csim} -c ${mkVisParset} > ${mkVisLog}

EOF

latestID=`qsub ${depend} ${qsubfile}`

echo "Running csimulator for pointing ${POINTING} with 1934-638 component, producing measurement set ${ms}: ID=${latestID}"





