#!/usr/bin/env bash
##############################################################################
# Spectral Line Imaging
##############################################################################

#
# Create the qsub file to image each channel individually
#
cat > cimager-spectral-line.sbatch << EOF
#!/bin/bash
#SBATCH --time=24:00:00
#SBATCH --ntasks=5504
#SBATCH --ntasks-per-node=16
#SBATCH --job-name sl-img
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cat > config/simager.in << EOF_INNER

Simager.dataset                                = ../input/dc1a.ms
#
Simager.Images.name                            = image.i.cube.spectral
Simager.Images.shape                           = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Simager.Images.frequency                       = [1.270e9,1.270e9]
Simager.Images.cellsize                        = [${IMAGING_CELLSIZE}, ${IMAGING_CELLSIZE}]
Simager.Images.direction                       = ${IMAGING_DIRECTION}
#
Simager.gridder.snapshotimaging                 = true
Simager.gridder.snapshotimaging.wtolerance      = 1500
Simager.gridder                                 = AWProject
Simager.gridder.AWProject.wmax                  = 1500
Simager.gridder.AWProject.nwplanes              = 11
Simager.gridder.AWProject.oversample            = 4
Simager.gridder.AWProject.diameter              = 12m
Simager.gridder.AWProject.blockage              = 2m
Simager.gridder.AWProject.maxfeeds              = 36
Simager.gridder.AWProject.maxsupport            = 512
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

# Apply calibration (untested so disabled)
Simager.calibrate                               = ${DO_CALIBRATION}
Simager.calibaccess                             = table
Simager.calibaccess.table                       = ${CALOUTPUT}
Simager.calibrate.scalenoise                    = true
Simager.calibrate.allowflag                     = true
EOF_INNER

LOGFILE=${LOGDIR}/simager-spectral-\${SLURM_JOB_ID}.log

lfs setstripe -c 4 .

# Now run the simager
aprun -B \${ASKAP_ROOT}/Code/Components/CP/simager/current/apps/simager.sh -c config/simager.in >> \${LOGFILE}
ERR=\$?
if [ \${ERR} -ne 0 ]; then
    echo "Error: simager returned error code \${ERR}"
    exit 1
fi
EOF

# Build dependencies
unset DEPENDS
if [ "${SBATCH_CAL}" ]; then
    DEPENDS="afterok:${SBATCH_CAL}"
fi

# Submit the jobs
echo "Spectral Line Imaging: Submitting"

unset DEPENDS
if [ "${SBATCH_CAL}" ]; then
    DEPENDS="afterok:${SBATCH_CAL}"
    SBATCH_SPECTRAL=`qsubmit cimager-spectral-line.sbatch`
else
    SBATCH_SPECTRAL=`qsubmit cimager-spectral-line.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_SPECTRAL}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_SPECTRAL}"
