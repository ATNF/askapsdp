#!/bin/bash -l


image=${imagebase}.BEAM${POINTING}

pbstag="cimSci${POINTING}"
qsubfile=cim_Science_BEAM${POINTING}.qsub
cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=${CONT_CLEAN_MPPWIDTH}
#PBS -l mppnppn=${CONT_CLEAN_MPPNPPN}
#PBS -N ${pbstag}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cim=${cim}

cd \$PBS_O_WORKDIR

imParset=${parsetdir}/cimScience-BEAM${POINTING}-\${PBS_JOBID}.in
imLogfile=${logdir}/cim-Science-BEAM${POINTING}-\${PBS_JOBID}.log

cat > \${imParset} << EOF_INNER
Cimager.dataset                                 = ${coarseMS}
Cimager.Feed                                    = ${POINTING}
#
Cimager.nworkergroups                           = 3
# Each worker will read a single channel selection
Cimager.Channels                                = [1, %w]
#
Cimager.Images.Names                            = [${image}]
Cimager.Images.shape                            = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Cimager.Images.cellsize                         = [${IMAGING_CELLSIZE},${IMAGING_CELLSIZE}]
Cimager.Images.${image}.frequency          = [${CONT_CLEAN_FREQ},${CONT_CLEAN_FREQ}]
Cimager.Images.${image}.nchan              = 1
Cimager.Images.${image}.direction          = ${IMAGING_DIRECTION}
Cimager.Images.${image}.nterms             = 2
Cimager.Images.writeAtMajorCycle                = true
#
Cimager.visweights                              = MFS
Cimager.visweights.MFS.reffreq                  = ${CONT_CLEAN_FREQ}
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = ${IMAGING_WMAX}
Cimager.gridder.AWProject.nwplanes              = ${IMAGING_NWPLANES}
Cimager.gridder.AWProject.oversample            = ${IMAGING_OVERSAMPLE}
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 9
Cimager.gridder.AWProject.maxsupport            = ${IMAGING_MAXSUP}
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 5000
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = [0, 3, 10, 30]
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.tolerance                  = 0.01
Cimager.solver.Clean.weightcutoff               = zero
Cimager.solver.Clean.weightcutoff.clean         = false
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 100
Cimager.threshold.minorcycle                    = [30%, 0.9mJy]
Cimager.threshold.majorcycle                    = 1mJy
Cimager.ncycles                                 = 5

Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = True
#
# Apply calibration
Cimager.calibrate                               = ${doCal}
Cimager.calibaccess                             = parset
Cimager.calibaccess.parset                      = caldata-combined.dat
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER



aprun -n ${CONT_CLEAN_MPPWIDTH} -N ${CONT_CLEAN_MPPNPPN}  \${cim} -c \${imParset} > \${imLogfile}

EOF

if [ $doSubmit == true ]; then

    latestID=`qsub $calDepend $qsubfile`
    echo "Running cimager for science field, imaging ${ms} to create ${image}.restored: ID=${latestID}"

fi

