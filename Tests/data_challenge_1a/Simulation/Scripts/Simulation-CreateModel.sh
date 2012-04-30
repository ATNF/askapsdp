#!/bin/bash -l

mkdir -p ${parsetdirCR}
mkdir -p ${logdirCR}
mkdir -p ${imagedir}
mkdir -p ${slicedir}

cd ${crdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}
touch ${now}

##############################
# Creation of images - one per worker
##############################

if [ $doCreateCR == true ]; then

#    crParsetBase=${parsetdirCR}/createModel-${now}.in
#    crParset=${parsetdirCR}/createModel-\${PBS_JOBID}.in
#
#    cat > $crParsetBase <<EOF
#####################
## AUTOMATICALLY GENERATED - DO NOT EDIT
#####################
##
#createFITS.filename         = !${modelimage}.fits
#createFITS.casaOutput       = true
#createFITS.fitsOutput       = false
#createFITS.nsubx            = ${nsubxCR}
#createFITS.nsuby            = ${nsubyCR}
#createFITS.flagWriteByChannel = true
#createFITS.createTaylorTerms = ${createTT_CR}
#createFITS.writeByNode      = false
#createFITS.sourcelist       = ${catdir}/${sourcelist}
#createFITS.database         = POSSUM
#createFITS.doContinuum      = true
#createFITS.posType          = deg
#createFITS.bunit            = Jy/pixel
#createFITS.dim              = 4
#createFITS.axes             = [${npix},${npix},${nstokes},${nchan}]
#createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
#createFITS.WCSimage.cunit   = [deg, deg, "",Hz]
#createFITS.WCSimage.crval   = [${ra}, ${dec}, ${stokesZero}, ${rfreq}]
#createFITS.WCSimage.crpix   = [${rpix},${rpix},${rstokes},${rchan}]
#createFITS.WCSimage.crota   = [0., 0., 0., 0.]
#createFITS.WCSimage.cdelt   = [-${delt},${delt}, ${dstokes}, ${chanw}]
#createFITS.WCSsources       = true
#createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
#createFITS.WCSsources.cunit = [deg, deg, "", Hz]
#createFITS.WCSsources.crval = [${raCat}, ${decCat}, ${stokesZero},  ${rfreq}]
#createFITS.WCSsources.crpix = [${rpix},${rpix},${rstokes},${rchan}]
#createFITS.WCSsources.crota = [0., 0., 0., 0.]
#createFITS.WCSsources.cdelt = [-${delt},${delt}, ${dstokes}, ${chanw}]
#createFITS.outputList       = false
#createFITS.addNoise         = false
#createFITS.doConvolution    = false
#createFITS.baseFreq         = ${basefreq}
#createFITS.flagSpectralInfo = false
#createFITS.PAunits          = rad
#createFITS.minMinorAxis     = 0.000100
#EOF
#
    numworkers=`echo $nsubxCR $nsubyCR | awk '{print $1*$2}'`
    numnodes=`echo $numworkers $workersPerNodeCR | awk '{print ($1+1.)*1./$2}'`
    numworkernodes=`echo $numworkers $workersPerNodeCR | awk '{print $1*1./$2}'`
    crQsub=${crdir}/${WORKDIR}/createModel.qsub
    cat > $crQsub <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=9:00:00
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1+${numworkernodes}:ncpus=12:mem=23GB:mpiprocs=${workersPerNodeCR}
#PBS -M matthew.whiting@csiro.au
#PBS -N DCmodelCF
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

module load openmpi
createFITS=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/install/bin/createFITS.sh

parset=${parsetdirCR}/createModel-\${PBS_JOBID}.in
cat > \$parset <<EOF_INNER
####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
createFITS.filename         = !${modelimage}.fits
createFITS.casaOutput       = true
createFITS.fitsOutput       = false
createFITS.nsubx            = ${nsubxCR}
createFITS.nsuby            = ${nsubyCR}
createFITS.flagWriteByChannel = true
createFITS.createTaylorTerms = ${createTT_CR}
createFITS.writeByNode      = false
createFITS.sourcelist       = ${catdir}/${sourcelist}
createFITS.database         = POSSUM
createFITS.doContinuum      = true
createFITS.posType          = deg
createFITS.bunit            = Jy/pixel
createFITS.dim              = 4
createFITS.axes             = [${npix}, ${npix}, ${nstokes}, ${nchan}]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [${ra}, ${dec}, ${stokesZero}, ${rfreq}]
createFITS.WCSimage.crpix   = [${rpix}, ${rpix}, ${rstokes}, ${rchan}]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-${delt}, ${delt}, ${dstokes}, ${chanw}]
createFITS.WCSsources       = true
createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSsources.cunit = [deg, deg, "", Hz]
createFITS.WCSsources.crval = [${raCat}, ${decCat}, ${stokesZero}, ${rfreq}]
createFITS.WCSsources.crpix = [${rpix}, ${rpix}, ${rstokes}, ${rchan}]
createFITS.WCSsources.crota = [0., 0., 0., 0.]
createFITS.WCSsources.cdelt = [-${delt}, ${delt}, ${dstokes}, ${chanw}]
createFITS.outputList       = false
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = ${basefreq}
createFITS.flagSpectralInfo = false
createFITS.PAunits          = rad
createFITS.minMinorAxis     = 0.000100
EOF_INNER

crLog=${logdirCR}/createModel-\${PBS_JOBID}.log

mpirun \$createFITS -inputs \$parset > \$crLog

EOF

    if [ $doSubmit == true ]; then
	crID=`qsub -h $crQsub`
	echo Running create job with ID $crID
	QSUB_JOBLIST="${QSUB_JOBLIST} ${crID}"
	depend="-W depend=afterok:${crID}"
    fi

fi

##############################
# Make slices
##############################

if [ $doSliceCR == true ]; then

    slQsub=${crdir}/${WORKDIR}/cubeslice-continuum.qsub
    cat > $slQsub <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=4:00:00
#PBS -l select=1:ncpus=1:mem=8GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N sliceCont
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
cubeslice=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/install/bin/cubeslice.sh

slLog=${logdirCR}/cubeslice-continuum-\${PBS_JOBID}.log

# make the models for each of the workers that hold the right number of channels
echo Extracting chunks from cube \`date\` >> \$slLog
CHUNK=0
chanPerMS=${chanPerMSchunk}
NCHUNKS=${numMSchunks}
startChan=0
while [ \$CHUNK -lt \$NCHUNKS ]; do
    chanPerMS=\`echo \$CHUNK ${nchan} \$chanPerMS | awk '{if(\$3>\$2-\$1*\$3) print \$2-\$1*\$3; else print \$3}'\`
    chunkcube=${slicebase}\${CHUNK}
    SECTION=\`echo \$startChan \$chanPerMS  | awk '{printf "[*,*,*,%d:%d]",\$1,\$1+\$2-1}'\`
    echo "\$CHUNK: Saving model image \$chunkcube with \$chanPerMS channels, using section \$SECTION" >> \$slLog
    \$cubeslice -s \$SECTION $modelimage \$chunkcube
    err=\$?
    if [ \$err != 0 ]; then
       exit \$err
    fi
    CHUNK=\`expr \$CHUNK + 1\`
    startChan=\`expr \$startChan + \$chanPerMS\`
done
exit \$err

EOF

    if [ $doSubmit == true ]; then

	slID=`qsub ${depend} ${slQsub}`
	echo Running cubeslice job with ID $slID
	depend="-W depend=afterok:${slID}"

    fi

fi

cd ${BASEDIR}
