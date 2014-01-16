#!/usr/bin/env bash
##############################################################################
# Continuum Imaging (Dirty)
##############################################################################

cat > cimager-cont-dirty.qsub << EOF
#!/bin/bash
##PBS -W group_list=${QUEUEGROUP}
#PBS -l mppwidth=${CONT_DIRTY_MPPWIDTH}
#PBS -l mppnppn=${CONT_DIRTY_MPPNPPN}
#PBS -l walltime=02:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-dirty
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

parset=${CONFIGDIR}/cimager-cont-dirty-\${PBS_JOBID}.in
logfile=${LOGDIR}/cimager-cont-dirty-\${PBS_JOBID}.log

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
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
    QSUB_CONTDIRTY=`qsubmit cimager-cont-dirty.qsub`
elif [ "${QSUB_MSSPLIT}" ]; then
    DEPENDS="afterok:${QSUB_MSSPLIT}"
    QSUB_CONTDIRTY=`qsubmit cimager-cont-dirty.qsub`
else
    QSUB_CONTDIRTY=`qsubmit cimager-cont-dirty.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTDIRTY}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CONTDIRTY}"
