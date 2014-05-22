#!/usr/bin/env bash
##############################################################################
# Continuum Cube Imaging (Clean)
##############################################################################

cat > simager-cont-cube-clean.sbatch << EOF
#!/bin/bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=305
#SBATCH --ntasks-per-node=10
#SBATCH --job-name contcube-clean
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cat > config/simager-cont-cube-clean.in << EOF_INNER

Simager.dataset                                = MS/coarse_chan.ms
#
Simager.Images.name                            = image.i.cube.clean
Simager.Images.shape                           = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Simager.Images.frequency                       = [1.270e9,1.270e9]
Simager.Images.cellsize                        = [${IMAGING_CELLSIZE}, ${IMAGING_CELLSIZE}]
Simager.Images.direction                       = ${IMAGING_DIRECTION}
#
Simager.gridder.snapshotimaging                 = true
Simager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Simager.gridder                                 = AWProject
Simager.gridder.AWProject.wmax                  = ${IMAGING_WMAX}
Simager.gridder.AWProject.nwplanes              = 33
Simager.gridder.AWProject.oversample            = 4
Simager.gridder.AWProject.diameter              = 12m
Simager.gridder.AWProject.blockage              = 2m
Simager.gridder.AWProject.maxfeeds              = 36
Simager.gridder.AWProject.maxsupport            = ${IMAGING_MAXSUP}
Simager.gridder.AWProject.variablesupport       = true
Simager.gridder.AWProject.offsetsupport         = true
Simager.gridder.AWProject.frequencydependent    = true
#
Simager.solver                                  = Clean
Simager.solver.Clean.algorithm                  = BasisfunctionMFS
Simager.solver.Clean.niter                      = 1000
Simager.solver.Clean.gain                       = 0.5
Simager.solver.Clean.scales                     = [0, 3, 10, 30]
Simager.solver.Clean.verbose                    = False
Simager.solver.Clean.psfwidth                   = 512
Simager.solver.Clean.logevery                   = 100
Simager.threshold.minorcycle                    = [10%, 10mJy]
Simager.threshold.majorcycle                    = 20mJy
Simager.ncycles                                 = 3
#
Simager.preconditioner.Names                    = [Wiener, GaussianTaper]
Simager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Simager.preconditioner.Wiener.robustness        = 0.0
Simager.preconditioner.Wiener.taper             = 64
#
Simager.restore                                 = true
Simager.restore.beam                            = fit

# Apply calibration
Simager.calibrate                               = ${DO_CALIBRATION}
Simager.calibaccess                             = table
Simager.calibaccess.table                       = ${CALOUTPUT}
Simager.calibrate.scalenoise                    = true
Simager.calibrate.allowflag                     = true
EOF_INNER

LOGFILE=${LOGDIR}/simager-cont-cube-clean-\${SLURM_JOB_ID}.log

lfs setstripe -c 4 .

# Now run the simager
aprun -n 305 -N 10 \${ASKAP_ROOT}/Code/Components/CP/simager/current/apps/simager.sh -c config/simager-cont-cube-clean.in > \${LOGFILE}
ERR=\$?
if [ \${ERR} -ne 0 ]; then
    echo "Error: simager returned error code \${ERR}"
    exit 1
fi
EOF

# Submit the jobs
echo "Continuum Cube Imager (Clean): Submitting"

unset DEPENDS
if [ "${SBATCH_CAL}" ]; then
    DEPENDS="afterok:${SBATCH_CAL}"
    SBATCH_CUBE_CLEAN=`qsubmit simager-cont-cube-clean.sbatch`
elif [ "${SBATCH_MSSPLIT}" ]; then
    DEPENDS="afterok:${SBATCH_MSSPLIT}"
    SBATCH_CONTCLEAN=`qsubmit simager-cont-cube-clean.sbatch`
else
    SBATCH_CUBE_CLEAN=`qsubmit imager-cont-cube-clean.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_CUBE_CLEAN}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_CUBE_CLEAN}"
