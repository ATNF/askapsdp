#!/usr/bin/env bash
##############################################################################
# Sky Model Image Generation
##############################################################################

SKYMODEL_OUTPUT=skymodel.image.taylor

cat > cmodel.sbatch << EOF
#!/bin/bash
#SBATCH --ntasks=20
#SBATCH --time=00:15:00
#SBATCH --job-name cmodel
#SBATCH --no-requeue
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cat > ${CONFIGDIR}/cmodel.in << EOF_INNER
# The below specifies the GSM source is a votable
Cmodel.gsm.database       = votable
Cmodel.gsm.file           = ${INPUT_SKYMODEL_XML}
Cmodel.gsm.ref_freq       = ${SKYMODEL_REFFREQ}

# General parameters
Cmodel.bunit              = Jy/pixel
Cmodel.frequency          = ${SKYMODEL_FREQ}
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

aprun -B -ss \${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/cmodel.sh -c ${CONFIGDIR}/cmodel.in > ${LOGDIR}/cmodel-\${SLURM_JOB_ID}.log
EOF

# Submit job
if [ ! -e ${SKYMODEL_OUTPUT}.0 ]; then
    echo "Sky Model Image: Submitting task"
    SBATCH_CMODEL=`qsubmit cmodel.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_CMODEL}"
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_CMODEL}"
else
    echo "Sky Model Image: Skipping task - Output already exists"
fi
