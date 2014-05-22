#!/usr/bin/env bash
##############################################################################
# Continuum Imaging (Dirty)
##############################################################################

cat > cimager-cont-dirty.sbatch << EOF
#!/bin/bash
#SBATCH --ntasks=${CONT_DIRTY_MPPWIDTH}
#SBATCH --ntasks-per-node=${CONT_DIRTY_MPPNPPN}
#SBATCH --time=02:00:00
##SBATCH --mail-user first.last@csiro.au
#SBATCH --job-name cont-dirty
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

parset=${CONFIGDIR}/cimager-cont-dirty-\${SLURM_JOB_ID}.in
logfile=${LOGDIR}/cimager-cont-dirty-\${SLURM_JOB_ID}.log

cat > \${parset} << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan.ms

# Each worker will read a single channel selection
Cimager.Channels                                = [1, %w]

Cimager.Images.Names                            = [image.i.dirty]
Cimager.Images.shape                            = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Cimager.Images.cellsize                         = [${IMAGING_CELLSIZE},${IMAGING_CELLSIZE}]
Cimager.Images.image.i.dirty.frequency          = [${CONT_DIRTY_FREQ},${CONT_DIRTY_FREQ}]
Cimager.Images.image.i.dirty.nchan              = 1
Cimager.Images.image.i.dirty.direction          = ${IMAGING_DIRECTION}
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = ${IMAGING_WMAX}
Cimager.gridder.AWProject.nwplanes              = 129
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = ${IMAGING_MAXSUP}
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = ${IMAGING_EQUALISE}
#
# Apply calibration
Cimager.calibrate                               = ${DO_CALIBRATION}
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
#Cimager.calibaccess                             = parset
#Cimager.calibaccess.parset                      = result.dat
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

aprun -n ${CONT_DIRTY_MPPWIDTH} -N ${CONT_DIRTY_MPPNPPN} -ss \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -c \${parset} > \${logfile}
EOF

## Submit the job
#echo "Continuum Imager (Dirty): Submitting"

unset DEPENDS
if [ "${SBATCH_CAL}" ]; then
    DEPENDS="afterok:${SBATCH_CAL}"
    SBATCH_CONTDIRTY=`qsubmit cimager-cont-dirty.sbatch`
elif [ "${SBATCH_MSSPLIT}" ]; then
    DEPENDS="afterok:${SBATCH_MSSPLIT}"
    SBATCH_CONTDIRTY=`qsubmit cimager-cont-dirty.sbatch`
else
    SBATCH_CONTDIRTY=`qsubmit cimager-cont-dirty.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_CONTDIRTY}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_CONTDIRTY}"
