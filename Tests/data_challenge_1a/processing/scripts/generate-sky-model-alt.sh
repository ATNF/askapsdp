#!/usr/bin/env bash
##############################################################################
# Sky Model Image Generation
##############################################################################

SKYMODEL_OUTPUT=skymodel.image.taylor

cat > createFITS.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=11:mem=23GB:mpiprocs=11
#PBS -l walltime=00:15:00
##PBS -M first.last@csiro.au
#PBS -N cmodel
#PBS -m a
#PBS -j oe
#PBS -r n
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/createFITS-\${PBS_JOBID}.in << EOF_INNER
createFITS.filename    = !${INPUT_SKYMODEL}.fits
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
createFITS.WCSimage.crval = [187.5, -45.0, 1, 1.271e9]
createFITS.WCSimage.crpix = [1780, 1780, 0, 0]
createFITS.WCSimage.crota = [0, 0, 0, 0]
createFITS.WCSimage.cdelt = [2.5342778e-3, 2.5324778e-3, 1, -1.e6]
createFITS.WCSsources = false
createFITS.outputList = false
createFITS.addNoise = false
createFITS.doConvolution = false
createFITS.baseFreq = 1.271e9
createFITS.flagSpectralInfo = false
createFITS.PAunits = deg
createFITS.minMinorAxis = 0.0001
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/install/bin/createFITS.sh -inputs ${CONFIGDIR}/createFITS-\${PBS_JOBID}.in > ${LOGDIR}/createFITS-\${PBS_JOBID}.log
EOF

# Submit job
if [ ! -e ${SKYMODEL_OUTPUT}.0 ]; then
    echo "Sky Model Image: Submitting task"
    QSUB_CMODEL=`qsubmit createFITS.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CMODEL}"
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CMODEL}"
else
    echo "Sky Model Image: Skipping task - Output already exists"
fi
