#!/bin/bash -l

selavy=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/selavy.sh
askapconfig=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

mkdir -p $scriptdirSM
mkdir -p $logdirSM
mkdir -p $subimagedirSM
mkdir -p $parsetdirSM

cd ${smdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}
touch ${now}

dependSM=${depend}


####################
# Smooth the models

if [ $doSmoothSM == true ]; then

    smoothSbatch=${smdir}/${WORKDIR}/smoothModels.sbatch

    if [ $doFlatSpectrum == true ]; then

	cat > $smoothSbatch <<EOF
#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name smoothTaylor
#SBATCH --mail-type=ALL
#SBATCH --no-requeue

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

smoothScript=${scriptdirSM}/smoothModels-\${SLURM_JOB_ID}.py
cat > \${smoothScript} <<EOF_INNER
#!/bin/env python

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
# CASA script to create a full-size continuum model by combining the 455 subcubes

modelInChunks = '${writeByNode}'
baseimage = '${baseimage}'
modelIm='${modelimage}'
smoothIm='${baseimage}-smooth'

ia.open(modelIm)
ia.setbrightnessunit('Jy/pixel')
ia.convolve2d(outfile=smoothIm,major='${smoothBmaj}arcsec',minor='${smoothBmin}arcsec',pa='${smoothBpa}deg')
ia.close()

EOF_INNER

output=${logdirSM}/smoothModels-\${SLURM_JOB_ID}.log
echo Running casa script to smooth  Taylor term images  > \$output
casapy --nologger --log2term -c \$smoothScript >> \$output
exit \$?
EOF

    else 

	cat > $smoothSbatch <<EOF
#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name smoothTaylor
#SBATCH --mail-type=ALL
#SBATCH --no-requeue

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

smoothScript=${scriptdirSM}/smoothModels-\${SLURM_JOB_ID}.py
cat > \${smoothScript} <<EOF_INNER
#!/bin/env python

import fnmatch
import numpy as np
import os

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
# CASA script to create a full-size continuum model by combining the 455 subcubes

modelInChunks = '${writeByNode}'
baseimage = '${baseimage}'
for t in range(3):

    modelIm='${modelimage}.taylor.%d'%t
    smoothIm='${baseimage}-smooth.taylor.%d'%t

############
# Need to make single images for create-by-chunk base - taylor-term
# images have been created in chunks, so need to read them and paste
# into full taylor-term image at appropriate location.
    if modelInChunks == "true" :
        goodfiles=[]
        for file in os.listdir('${chunkdir}'):
            if fnmatch.fnmatch(file,'%s_w*__.taylor.%d'%(baseimage,t)):
                goodfiles.append(file)
        goodfiles.sort()
        
        ia.open('${chunkdir}/%s'%goodfiles[0])
        crec=ia.coordsys().torecord()
        crec['direction0']['crpix']=np.array([${npix}/2.,${npix}/2.])
        ia.close()
        ia.newimagefromshape(outfile=modelIm,shape=[${npix},${npix},1,1],csys=crec)
        
        for file in goodfiles:
            offset=np.array(file.split('__')[1].split('_'),dtype=int)
            ia.open('${chunkdir}/%s'%file)
            shape=ia.shape()
            blc=np.zeros(len(shape),dtype=int).tolist()
            trc=(np.array(shape,dtype=int)-1).tolist()
            chunk=ia.getchunk(blc=blc,trc=trc)
            ia.close()
            ia.open(modelIm)
            ia.putchunk(pixels=chunk,blc=offset.tolist())
            ia.close()
###########

    ia.open(modelIm)
    ia.setbrightnessunit('Jy/pixel')
    ia.convolve2d(outfile=smoothIm,major='${smoothBmaj}arcsec',minor='${smoothBmin}arcsec',pa='${smoothBpa}deg')
    ia.close()

EOF_INNER

output=${logdirSM}/smoothModels-\${SLURM_JOB_ID}.log
echo Running casa script to smooth  Taylor term images  > \$output
casapy --nologger --log2term -c \$smoothScript >> \$output
exit \$?
EOF

    fi
    
    if [ $doSubmit == true ]; then

	smoothID=`sbatch ${dependSM} $smoothSbatch | awk '{print $4}'`
	echo Submitted job $smoothID to smooth Taylor term images
	if [ "$dependSM" == "" ]; then
	    dependSM="--dependency=afterok:${smoothID}"
	else
	    dependSM="${dependSM}:${smoothID}"
	fi

    fi

fi


#####################
# Find & fit sources

if [ $doSF_SM == true ]; then

    if [ $doFlatSpectrum == true ]; then
	smoothIm="${baseimage}-smooth"
    else
	smoothIm="${baseimage}-smooth.taylor.0"
    fi

    selavySbatch=${smdir}/${WORKDIR}/selavy-smooth.sbatch
    cat > $selavySbatch <<EOF
#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name selavyTaylor
#SBATCH --mail-type=ALL
#SBATCH --no-requeue

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

export AIPSPATH=${AIPSPATH}

selavyParset=${parsetdirSM}/selavy-smooth-\${SLURM_JOB_ID}.in
cat > \${selavyParset} <<EOF_INNER
####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
Selavy.imageFile = ${smoothIm}
Selavy.threshold = ${SFthresh}
Selavy.flagGrowth = ${SFflagGrowth}
Selavy.growthThreshold = ${SFgrowthThresh}
#Selavy.nsubx = ${SFnsubx}
#Selavy.nsuby = ${SFnsuby}
Selavy.doFit = true
Selavy.fitJustDetection = true
Selavy.Fitter.useNoise = false
Selavy.Fitter.noiseLevel = 1.e-3
Selavy.Fitter.numSubThresholds = 1000
Selavy.findSpectralIndex = true
EOF_INNER

output=${logdirSM}/selavy-smooth-\${SLURM_JOB_ID}.log
aprun -B $selavy -c \${selavyParset} > \${output}

exit \$?

EOF

    if [ $doSubmit == true ]; then

	selavyID=`sbatch ${dependSM} $selavySbatch | awk '{print $4}'`
	echo Submitting Selavy job with ID $selavyID
	if [ "$dependSM" == "" ]; then
	    dependSM="--dependency=afterok:${selavyID}"
	else
	    dependSM="${dependSM}:${selavyID}"
	fi

    fi

fi

#####################
# Find & fit sources

if [ $doComparisonSM == true ]; then

    # make a comparison image - single channel only
    # then use imagecalc in casacore to find the difference between the two

    if [ $doFlatSpectrum == true ]; then
	smoothIm="${baseimage}-smooth"
    else
	smoothIm="${baseimage}-smooth.taylor.0"
    fi

    modelcompImage=DCmodelcomp-singlechan
    modelcompDiff=DCmodelcomp-singlechan-residual
    nsubx=1
    nsuby=1
    npix=3560
    rpix=1780
    nstokes=1
    nchan=1
    rchan=1
    rfreq=1.421e9
    chanw=18.31055e3
    cellsize=9.1234
    delt=`echo $cellsize | awk '{print $1/3600.}'`

    modelcompSbatch=${smdir}/${WORKDIR}/modelComparison.sbatch
    cat > $modelcompSbatch <<EOF
#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name modelComp
#SBATCH --mail-type=ALL
#SBATCH --no-requeue

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=${AIPSPATH}
export CASACOREDIR=\${ASKAP_ROOT}/3rdParty/casacore/casacore-1.6.0a
createFITS=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh
imagecalc=\${CASACOREDIR}/install/bin/imagecalc

modelcompParset=${parsetdirSM}/modelcomp-\${SLURM_JOB_ID}.in
cat > \${modelcompParset} <<EOF_INNER
####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
createFITS.filename         = !${modelcompImage}.fits
createFITS.casaOutput       = true
createFITS.fitsOutput       = false
createFITS.nsubx            = ${nsubx}
createFITS.nsuby            = ${nsuby}
createFITS.writeByNode      = true
createFITS.sourcelist       = selavy-fitResults.txt
createFITS.database         = Selavy
createFITS.Selavyimage      = ${baseimage}-smooth.taylor.0
createFITS.doContinuum      = true
createFITS.posType          = deg
createFITS.bunit            = Jy/pixel
createFITS.dim              = 4
createFITS.axes             = [${npix},${npix},${nstokes},${nchan}]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "",Hz]
createFITS.WCSimage.crval   = [187.5, -45., 1., ${rfreq}]
createFITS.WCSimage.crpix   = [${rpix},${rpix},1,${rchan}]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-${delt},${delt}, 1, -${chanw}]
createFITS.WCSsources       = false
createFITS.outputList       = false
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = $rfreq
createFITS.flagSpectralInfo = false
createFITS.PAunits          = deg
createFITS.minMinorAxis     = 0.000100
EOF_INNER

output=${logdirSM}/modelcomp-\${SLURM_JOB_ID}.log
aprun -B \$createFITS -c \${modelcompParset} > \${output}
err=\$?
if [ \$err -ne 0 ]; then
    exit \$?
fi

. \${CASACOREDIR}/init_package_env.sh
\${imagecalc} in="'${smoothIm}' - '${modelcompImage}'" out='${modelcompDiff}'
err=\$?
exit \$err

EOF

    if [ $doSubmit == true ]; then
	
	modelcompID=`sbatch $dependSM $modelcompSbatch | awk '{print $4}'`
	echo Submitting Model Comparison job with ID $modelcompID

    fi

fi

cd ${BASEDIR}
