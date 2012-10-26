#!/usr/bin/env bash
##############################################################################
# Sky Model Image Generation
##############################################################################

SKYMODEL_OUTPUT=skymodel.image.taylor

cat > cmodel.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=12:mem=23GB:mpiprocs=12
#PBS -l walltime=00:15:00
##PBS -M first.last@csiro.au
#PBS -N cmodel
#PBS -m a
#PBS -j oe
#PBS -r n
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/cmodel.in << EOF_INNER
# The below specifies the GSM source is a votable
Cmodel.gsm.database       = votable
Cmodel.gsm.file           = ${INPUT_SKYMODEL_XML}
Cmodel.gsm.ref_freq       = 1.421GHz

# General parameters
Cmodel.bunit              = Jy/pixel
Cmodel.frequency          = 1.421GHz
Cmodel.increment          = 300MHz
Cmodel.flux_limit         = 1mJy
Cmodel.shape              = [3560, 3560]
Cmodel.cellsize           = [9.1234arcsec, 9.1234arcsec]
Cmodel.direction          = [12h30m00.00, -45.00.00.00, J2000]
Cmodel.stokes             = [I]
Cmodel.nterms             = 3
Cmodel.batchsize          = 10

# Output specific parameters
Cmodel.output             = casa
Cmodel.filename           = ${SKYMODEL_OUTPUT}
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/cmodel.sh -c ${CONFIGDIR}/cmodel.in > ${LOGDIR}/cmodel-\${PBS_JOBID}.log
EOF

# Submit job
if [ ! -e ${SKYMODEL_OUTPUT}.0 ]; then
    echo "Sky Model Image: Submitting task"
    QSUB_CMODEL=`qsubmit cmodel.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CMODEL}"
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CMODEL}"
else
    echo "Sky Model Image: Skipping task - Output already exists"
fi
