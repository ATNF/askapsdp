#!/usr/bin/env bash
##############################################################################
# Continuum Cube Imaging (Dirty)
##############################################################################

cat > simager-cont-cube-dirty.sbatch << EOF
#!/bin/bash
#SBATCH --time=02:00:00
#SBATCH --ntasks=305
#SBATCH --ntasks-per-node=16
#SBATCH --job-name contcube-dirty
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cat > config/simager-cont-cube-dirty.in << EOF_INNER

Simager.dataset                                = MS/coarse_chan.ms
#
Simager.Images.name                            = image.i.cube.dirty
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
Simager.solver                                  = Dirty
Simager.solver.Dirty.tolerance                  = 0.1
Simager.solver.Dirty.verbose                    = True
Simager.ncycles                                 = 0
#
Simager.preconditioner.Names                    = [Wiener, GaussianTaper]
Simager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Simager.preconditioner.Wiener.robustness        = 0.0
Simager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit

# Apply calibration
Simager.calibrate                               = ${DO_CALIBRATION}
Simager.calibaccess                             = table
Simager.calibaccess.table                       = ${CALOUTPUT}
Simager.calibrate.scalenoise                    = true
Simager.calibrate.allowflag                     = true
EOF_INNER

LOGFILE=${LOGDIR}/simager-cont-cube-dirty-\${SLURM_JOB_ID}.log

lfs setstripe -c 4 .

# Now run the simager
aprun -n 305 -N 16 -ss \${ASKAP_ROOT}/Code/Components/CP/simager/current/apps/simager.sh -c config/simager-cont-cube-dirty.in > \${LOGFILE}
ERR=\$?
if [ \${ERR} -ne 0 ]; then
    echo "Error: simager returned error code \${ERR}"
    exit 1
fi
EOF

# Submit the jobs
echo "Continuum Cube Imager (Dirty): Submitting"

unset DEPENDS
if [ "${SBATCH_CAL}" ]; then
    DEPENDS="afterok:${SBATCH_CAL}"
    SBATCH_CUBE_DIRTY=`qsubmit simager-cont-cube-dirty.sbatch`
elif [ "${SBATCH_MSSPLIT}" ]; then
    DEPENDS="afterok:${SBATCH_MSSPLIT}"
    SBATCH_CONTCLEAN=`qsubmit simager-cont-cube-dirty.sbatch`
else
    SBATCH_CUBE_DIRTY=`qsubmit imager-cont-cube-dirty.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_CUBE_DIRTY}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_CUBE_DIRTY}"
