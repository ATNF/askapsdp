##############################################################################
# Continuum Imaging (Clean)
##############################################################################

cat > cimager-cont-clean.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=23GB:mpiprocs=1+50:ncpus=6:mem=23GB:mpiprocs=6
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-clean
#PBS -m a
#PBS -j oe

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/cimager-cont-clean.in << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan_%w.ms

Cimager.Images.Names                            = [image.i.clean]
Cimager.Images.shape                            = [3584,3584]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.image.i.clean.frequency          = [1.420e9,1.420e9]
Cimager.Images.image.i.clean.nchan              = 1
Cimager.Images.image.i.clean.direction          = [12h30m00.00, -45.00.00.00, J2000]
Cimager.Images.image.i.clean.writeAtMajorCycle  = true
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 2000
Cimager.gridder.AWProject.nwplanes              = 7
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 2048
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 1000
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = [0, 3, 10, 30]
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 100
Cimager.threshold.minorcycle                    = [1%, 1.0mJy]
Cimager.threshold.majorcycle                    = 10mJy
Cimager.ncycles                                 = 3
#
Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 100
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
Cimager.restore.equalise                        = True
#
# Apply calibration
#Cimager.calibrate                               = true
Cimager.calibrate                               = false
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs ${CONFIGDIR}/cimager-cont-clean.in > ${LOGDIR}/cimager-cont-clean.log
EOF

if [ ! $DRYRUN ]; then
echo "Continuum Imager (Clean): Submitting task"
if [ ${QSUB_CCAL} ]; then
    QSUB_CONTCLEAN=`${QSUB_CMD} -W depend=afterok:${QSUB_CCAL} cimager-cont-clean.qsub`
else
    QSUB_CONTCLEAN=`${QSUB_CMD} cimager-cont-clean.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTCLEAN}"
fi
else
echo "Continuum Imager (Clean): Dry Run Only"
fi
