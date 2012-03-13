#!/bin/bash -l

mkdir -p ${parsetdirCR}
mkdir -p ${logdirCR}
mkdir -p ${scriptdirCR}
mkdir -p ${imagedir}
mkdir -p ${slicedir}

cd ${crdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}


##############################
# Creation of images - one per worker
##############################

if [ $doCreateCR == true ]; then

    crParsetBase=${parsetdirCR}/createModel-${now}.in
    crParset=${parsetdirCR}/createModel-\${PBS_JOBID}.in

    cat > $crParsetBase <<EOF
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
createFITS.axes             = [${npix},${npix},${nstokes},${nchan}]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "",Hz]
createFITS.WCSimage.crval   = [187.5, -45., 1., ${rfreq}]
createFITS.WCSimage.crpix   = [${rpix},${rpix},1,${rchan}]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-${delt},${delt}, 1, -${chanw}]
createFITS.WCSsources       = true
createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSsources.cunit = [deg, deg, "", Hz]
createFITS.WCSsources.crval = [0., 0., 1.,  ${rfreq}]
createFITS.WCSsources.crpix = [${rpix},${rpix},1,${rchan}]
createFITS.WCSsources.crota = [0., 0., 0., 0.]
createFITS.WCSsources.cdelt = [-${delt},${delt}, 1, -${chanw}]
createFITS.outputList       = false
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = 1.42e9
createFITS.flagSpectralInfo = false
createFITS.PAunits          = rad
createFITS.minMinorAxis     = 0.000100
EOF

    numworkers=`echo $nsubxCR $nsubyCR | awk '{print $1*$2}'`
    numnodes=`echo $numworkers $workersPerNodeCR | awk '{print ($1+1.)*1./$2}'`
    numworkernodes=`echo $numworkers $workersPerNodeCR | awk '{print $1*1./$2}'`
    crQsub=${crdir}/createModel.qsub
    cat > $crQsub <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=9:00:00
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1+${numworkernodes}:ncpus=12:mem=23GB:mpiprocs=${workersPerNodeCR}
#PBS -M matthew.whiting@csiro.au
#PBS -N DCmodelCF
#PBS -m bea
#PBS -j oe

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

module load openmpi
export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
createFITS=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/install/bin/createFITS.sh

mv $crParsetBase $crParset
crLog=${logdirCR}/createModel-\${PBS_JOBID}.log

mpirun \$createFITS -inputs $crParset > \$crLog

EOF

    if [ $doSubmit == true ]; then
	crID=`qsub -h $crQsub`
	echo Running create job with ID $crID
	QSUB_JOBLIST="${QSUB_JOBLIST} ${crID}"
	depend="-W depend=afterok:${crID}"
    fi

fi

##############################
# Combine subimages into large model
##############################

if [ $doCombineCR == true ]; then

    comScriptBase=${scriptdirCR}/combineSubimages-continuum-${now}.py
    comScript=${scriptdirCR}/combineSubimages-continuum-\${PBS_JOBID}.py

    cat > $comScriptBase <<EOF
#!/bin/env python

from numpy import *
import os

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
# CASA script to create a full-size continuum model by combining the 455 subcubes

fullimage='${modelimage}'

imsize=${npix}
spsize=${nchan}
stsize=${nstokes}
outshape=[imsize,imsize,stsize,spsize]

cellsize=${cellsize}
chanw=${chanw}

ra=187.5
dec=-45.
freq=${rfreq}

makeIt=${createFullModelCR}
minWorker=${minWorkerCR}

if makeIt:
    ia.newimagefromshape(outfile=fullimage,shape=outshape,overwrite=True)
    ia.close()

ia.open(fullimage)
crec=ia.coordsys().torecord()

if makeIt:
    crec['direction0']['cdelt']=array([-cellsize,cellsize])*pi/180./3600.
    crec['direction0']['crpix']=array([imsize/2,imsize/2])*1.
    crec['direction0']['crval']=array([ra,dec])*pi/180.
    crec['direction0']['units']=array(["rad","rad"])
    crec['spectral2']['wcs']['cdelt']=-1.*chanw
    crec['spectral2']['wcs']['crpix']=1.
    crec['spectral2']['wcs']['crval']=freq
    ia.setcoordsys(crec)

ia.close()

baseref=crec['direction0']['crpix']

nworkers=${numworkers}

for im in range(minWorker,nworkers+1):

    #read the subimage
    input="%s_w%d"%(fullimage,im)

    ia.open(input)
    inshape=array(ia.shape())
    print(inshape)
    crecim=ia.coordsys().torecord()
    pixarr = ia.getregion()
    ia.close()

    #get the reference position from the coordsys. Use this to work out where to put the image's pixels in the full image
    refpix=crecim['direction0']['crpix']
    offset = baseref - refpix
    blc=array([int(offset[0]),int(offset[1]),0,0])
    trc=blc+inshape-1
    print(blc,trc)

    rgput = rg.box(blc=blc.tolist(), trc=trc.tolist())
    print rgput

    ia.open(fullimage)
    ia.putregion(pixels=pixarr,region=rgput, list=True)
    ia.close()
EOF

    comQsub=combineSubimages-continuum.qsub
    cat > $comQsub <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=12:00:00
#PBS -l select=1:ncpus=1:mem=23GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N combCont
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

mv $comScriptBase $comScript

output=${logdirCR}/combineSubimages-continuum-\${PBS_JOBID}.log
echo Running casa script to combine subimages into a single large cube  > \$output
casapy --nologger --log2term -c $comScript >> \$output
exit \$?
EOF

    if [ $doSubmit == true ]; then

	comID=`qsub ${depend} ${comQsub}`
	echo Running combine job with ID $comID
	if [ "$depend" == "" ]; then
	    depend="-W depend=afterok:${comID}"
	else
	    depend="${depend}:${comID}"
	fi

    fi

fi

##############################
# Make slices
##############################

if [ $doSliceCR == true ]; then

    slQsub=${crdir}/cubeslice-continuum.qsub
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
NCHUNKS=820
chanPerMS=20
startChan=0
while [ \$CHUNK -lt \$NCHUNKS ]; do
    if [ \$CHUNK == 819 ]; then
        chanPerMS=4
    fi
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
