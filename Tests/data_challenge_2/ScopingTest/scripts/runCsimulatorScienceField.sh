#!/bin/bash -l

. ${scriptdir}/getTags.sh

mkVisParset=parsets/csim-Science-${tag}.in
mkVisLog=logs/csim-Science-${tag}.log

direction=${dirlist[4]}

ms=${msbase}_Science_${mstag}.ms

spw="[1, ${nurefMHz} MHz, ${chanw} Hz, \"${pol}\"]"

cat > ${mkVisParset} << EOF_INNER
Csimulator.dataset                              =       ${ms}
#
Csimulator.stman.bucketsize                     =       2097152
#
Csimulator.sources.names                        =       [DCmodel]
Csimulator.sources.DCmodel.direction            =       ${direction}
Csimulator.sources.DCmodel.model                =       ${modelImage}
#
# Define the antenna locations, feed locations, and spectral window definitions
#
Csimulator.antennas.definition                   =       ${askapconfig}/BETAXYZ.in
Csimulator.feeds.definition                      =       ${askapconfig}/ASKAP9feeds.in
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
Csimulator.observe.scan0                         =       [DCmodel, thisSPWS, -6h, 6h]
#
Csimulator.gridder                               =       AWProject
Csimulator.gridder.padding                       =       1.
Csimulator.gridder.snapshotimaging               =       true
Csimulator.gridder.snapshotimaging.wtolerance    =       1000
Csimulator.gridder.AWProject.wmax               =       1000
Csimulator.gridder.AWProject.nwplanes           =      ${nw}
Csimulator.gridder.AWProject.oversample         =      ${os}
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

slurmtag="csimSci${tag}"
sbatchfile=csim_Science_${tag}.sbatch
cat > $sbatchfile <<EOF
#!/bin/bash -l
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name ${slurmtag}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

rm -rf $ms

aprun ${csim} -c ${mkVisParset} > ${mkVisLog}

EOF

latestID=`sbatch ${depend} ${sbatchfile} | awk '{print $4}'`

echo "Running csimulator for science field, producing measurement set ${ms}: ID=${latestID}"




