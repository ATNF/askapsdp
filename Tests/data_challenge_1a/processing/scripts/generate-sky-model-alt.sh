#!/usr/bin/env bash
##############################################################################
# Sky Model Image Generation
##############################################################################

SKYMODEL_OUTPUT=skymodel.image

cat > createFITS.sbatch << EOF
#!/bin/bash
#SBATCH --ntasks=11
#SBATCH --time=00:15:00
#SBATCH --job-name cmodel
#SBATCH --no-requeue
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cat > ${CONFIGDIR}/createFITS-\${SLURM_JOB_ID}.in << EOF_INNER
createFITS.filename    = !${SKYMODEL_OUTPUT}.fits
createFITS.casaOutput = true
createFITS.fitsOutput = false
createFITS.nsubx = 5
createFITS.nsuby = 2
createFITS.createTaylorTerms = true
createFITS.writeFullImage = false
createFITS.sourcelist =  ../input/skymodel-duchamp.txt
createFITS.database = Selavy
createFITS.useDeconvolvedSizes = true
createFTIS.doContinuum = true
createFITS.posType = deg
createFITS.bunit = Jy/pixel
createFITS.dim = 4
createFITS.axes = [3560, 3560, 1, 300]
createFITS.WCSimage.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit = [deg, deg, "", Hz]
createFITS.WCSimage.crval = [187.5, -45.0, 1, ${SKYMODEL_ALT_FREQ}]
createFITS.WCSimage.crpix = [1780, 1780, 0, 0]
createFITS.WCSimage.crota = [0, 0, 0, 0]
createFITS.WCSimage.cdelt = [2.5342778e-3, 2.5342778e-3, 1, -1.e6]
createFITS.WCSsources = false
createFITS.outputList = false
createFITS.addNoise = false
createFITS.doConvolution = false
createFITS.baseFreq = ${SKYMODEL_ALT_FREQ}
createFITS.flagSpectralInfo = false
createFITS.PAunits = deg
createFITS.minMinorAxis = 0.0001
EOF_INNER

aprun -n 11 \${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh -c ${CONFIGDIR}/createFITS-\${SLURM_JOB_ID}.in > ${LOGDIR}/createFITS-\${SLURM_JOB_ID}.log
EOF

# Submit job
if [ ! -e ${SKYMODEL_OUTPUT}.0 ]; then
    echo "Sky Model Image: Submitting task"
    SBATCH_CMODEL=`qsubmit createFITS.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_CMODEL}"
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_CMODEL}"
else
    echo "Sky Model Image: Skipping task - Output already exists"
fi
