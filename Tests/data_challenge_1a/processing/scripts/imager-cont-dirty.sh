#!/usr/bin/env bash
##############################################################################
# Continuum Imaging (Dirty)
##############################################################################

cat > cimager-cont-dirty.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=5:mem=23GB:mpiprocs=5+50:ncpus=6:mem=23GB:mpiprocs=6
#PBS -l walltime=04:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-dirty
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

parset=${CONFIGDIR}/cimager-cont-dirty-\${PBS_JOBID}.in
logfile=${LOGDIR}/cimager-cont-dirty-\${PBS_JOBID}.log

cat > \${parset} << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan_%w.ms

Cimager.Images.Names                            = [image.i.dirty]
Cimager.Images.shape                            = [3328,3328]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.image.i.dirty.frequency          = [1.420e9,1.420e9]
Cimager.Images.image.i.dirty.nchan              = 1
Cimager.Images.image.i.dirty.direction          = [12h30m00.00, -45.00.00.00, J2000]
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 1000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 1000
Cimager.gridder.AWProject.nwplanes              = 129
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 512
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = None
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = True
#
# Apply calibration
Cimager.calibrate                               = ${DO_CALIBRATION}
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs \${parset} > \${logfile}
EOF

# Submit the job
echo "Continuum Imager (Dirty): Submitting"

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
