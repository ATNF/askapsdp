##############################################################################
# Continuum Imaging (Clean)
##############################################################################

cat > cimager-cont-clean.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=23GB:mpiprocs=1+76:ncpus=4:mem=23GB:mpiprocs=4
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-clean
#PBS -m a
#PBS -j oe

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/cimager-cont-clean.in << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan_%w.ms

Cimager.Images.Names                            = [image.i.clean]
Cimager.Images.shape                            = [3328,3328]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.image.i.clean.frequency          = [1.420e9,1.420e9]
Cimager.Images.image.i.clean.nchan              = 1
Cimager.Images.image.i.clean.direction          = [12h30m00.00, -45.00.00.00, J2000]
Cimager.Images.image.i.clean.nterms             = 2
Cimager.Images.clean.writeAtMajorCycle          = true
#
Cimager.visweights                              = MFS
Cimager.visweights.MFS.reffreq                  = 1.420e9
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 2000
Cimager.gridder.AWProject.nwplanes              = 5
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
Cimager.solver.Clean.psfwidth                   = 1024
Cimager.solver.Clean.logevery                   = 100
Cimager.threshold.minorcycle                    = [10%, 10.0mJy]
Cimager.threshold.majorcycle                    = 20mJy
Cimager.ncycles                                 = 5
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
Cimager.calibrate                               = ${DO_CALIBRATION}
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs ${CONFIGDIR}/cimager-cont-clean.in > ${LOGDIR}/cimager-cont-clean.log
EOF

if [ "${DRYRUN}" == "false" ]; then
    echo "Continuum Imager (Clean): Submitting task"

    # Add dependencies
    unset DEPENDS
    if [ "${QSUB_CAL}" ] || [ "${QSUB_MSSPLIT}" ]; then
        if [ "${QSUB_CAL}" ]; then
            DEPENDS="-W depend=afterok:${QSUB_CAL}"
        else
            DEPENDS="-W depend=afterok:${QSUB_MSSPLIT}"
        fi
    fi

    # Submit the jobs
    if [ "${DEPENDS}" ]; then
        QSUB_CONTCLEAN=`${QSUB_CMD} ${DEPENDS} cimager-cont-clean.qsub`
    else
        QSUB_CONTCLEAN=`${QSUB_CMD} cimager-cont-clean.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTCLEAN}"
    fi

else
    echo "Continuum Imager (Clean): Dry Run Only"
fi
