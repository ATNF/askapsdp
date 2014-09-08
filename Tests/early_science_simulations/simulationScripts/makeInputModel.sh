#!/bin/bash -l


##############################
# Creation of images - one per worker
##############################

if [ $doCreateModel == true ]; then

    echo "In makeInputModel.sh : this runs a job to create the large input model."

    crSbatch=createModel.sbatch
    cat > $crSbatch <<EOF
#!/bin/bash -l
#SBATCH --time=12:00:00
#SBATCH --ntasks=${CREATORTASKS}
#SBATCH --nodes=${CREATORNODES}
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name DCmodelCF
#SBATCH --mail-type=ALL
#SBATCH --no-requeue

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
createFITS=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh

parset=${parsetdir}/createModel-\${SLURM_JOB_ID}.in
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
createFITS.posType          = ${posType}
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

crLog=${logdir}/createModel-\${SLURM_JOB_ID}.log

aprun -n ${CREATORTASKS} \$createFITS -c \$parset > \$crLog

EOF

    if [ $doSubmit == true ]; then
	crID=`sbatch --hold $crSbatch | awk '{print $4}'`
	echo Running create job with ID $crID
	SBATCH_JOBLIST="${SBATCH_JOBLIST} ${crID}"
	depend="-d afterok:${crID}"
    fi

fi
