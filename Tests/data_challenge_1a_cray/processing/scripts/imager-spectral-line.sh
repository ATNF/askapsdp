#!/usr/bin/env bash
##############################################################################
# Spectral Line Imaging
##############################################################################

#
# Create the qsub file to image each channel individually
#
cat > cimager-spectral-line.qsub << EOF
#!/bin/bash
#PBS -l walltime=12:00:00
#PBS -l mppwidth=5504
#PBS -l mppnppn=16
#PBS -N sl-img
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

cat > config/simager.in << EOF_INNER

Simager.dataset                                = ../input/dc1a.ms
#
Simager.Images.name                            = image.i.cube.spectral
Simager.Images.shape                           = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Simager.Images.frequency                       = [1.270e9,1.270e9]
Simager.Images.cellsize                        = [${IMAGING_CELLSIZE}, ${IMAGING_CELLSIZE}]
Simager.Images.direction                       = [12h30m00.00, -45.00.00.00, J2000]
#
Simager.gridder.snapshotimaging                 = true
Simager.gridder.snapshotimaging.wtolerance      = 2000
Simager.gridder                                 = AWProject
Simager.gridder.AWProject.wmax                  = 2000
#Simager.gridder.AWProject.nwplanes              = 129
Simager.gridder.AWProject.nwplanes              = 33
Simager.gridder.AWProject.oversample            = 4
Simager.gridder.AWProject.diameter              = 12m
Simager.gridder.AWProject.blockage              = 2m
Simager.gridder.AWProject.maxfeeds              = 36
Simager.gridder.AWProject.maxsupport            = 1024
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
Simager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Simager.preconditioner.Wiener.robustness        = 0.0
Simager.preconditioner.Wiener.taper             = 64

# Apply calibration (untested so disabled)
#Simager.calibrate                               = ${DO_CALIBRATION}
#Simager.calibaccess                             = table
#Simager.calibaccess.table                       = ${CALOUTPUT}
#Simager.calibrate.scalenoise                    = true
#Simager.calibrate.allowflag                     = true
EOF_INNER

LOGFILE=${LOGDIR}/cimager_spectral_\${PBS_JOBID}.log

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
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
fi

# Submit the jobs
echo "Spectral Line Imaging: Submitting"

unset DEPENDS
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
    QSUB_SPECTRAL=`qsubmit cimager-spectral-line.qsub`
else
    QSUB_SPECTRAL=`qsubmit cimager-spectral-line.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_SPECTRAL}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_SPECTRAL}"
