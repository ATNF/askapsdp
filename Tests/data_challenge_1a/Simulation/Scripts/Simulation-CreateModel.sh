#!/bin/bash -l

mkdir -p ${parsetdirCR}
mkdir -p ${logdirCR}
mkdir -p ${imagedir}
mkdir -p ${chunkdir}

cd ${crdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}
touch ${now}

##############################
# Creation of images - one per worker
##############################

if [ $doCreateCR == true ]; then

    crQsub=${crdir}/${WORKDIR}/createModel.qsub
    cat > $crQsub <<EOF
#!/bin/bash -l
#PBS -l walltime=12:00:00
#PBS -l mppwidth=${CREATORWIDTH}
#PBS -l mppnppn=${CREATORPPN}
#PBS -M matthew.whiting@csiro.au
#PBS -N DCmodelCF
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
createFITS=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh

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
createFITS.writeByNode      = ${writeByNode}
createFITS.sourcelist       = ${catdir}/${sourcelist}
createFITS.database         = ${databaseCR}
createFITS.sourcelisttype   = ${listtypeCR}
createFITS.useGaussians     = ${useGaussianComponents}
createFITS.verboseSources   = false
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
createFITS.WCSsources       = ${WCSsources}
createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSsources.cunit = [deg, deg, "", Hz]
createFITS.WCSsources.crval = [${raCat}, ${decCat}, ${stokesZero}, ${rfreq}]
createFITS.WCSsources.crpix = [${rpix}, ${rpix}, ${rstokes}, ${rchan}]
createFITS.WCSsources.crota = [0., 0., 0., 0.]
createFITS.WCSsources.cdelt = [-${delt}, ${delt}, ${dstokes}, ${chanw}]
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = ${basefreq}
createFITS.flagSpectralInfo = false
createFITS.PAunits          = ${PAunits}
createFITS.minMinorAxis     = 0.000100
EOF_INNER

crLog=${logdirCR}/createModel-\${PBS_JOBID}.log

aprun -n ${CREATORWIDTH} -N ${CREATORPPN} \$createFITS -c \$parset > \$crLog

EOF

    if [ $doSubmit == true ]; then
	crID=`qsub -h $crQsub`
	echo Running create job with ID $crID
	QSUB_JOBLIST="${QSUB_JOBLIST} ${crID}"
	depend="-W depend=afterok:${crID}"
    fi

fi


cd ${BASEDIR}
