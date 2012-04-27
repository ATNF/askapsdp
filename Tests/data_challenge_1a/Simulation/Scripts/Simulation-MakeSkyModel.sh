#!/bin/bash -l

cduchamp=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/install/bin/cduchamp.sh
askapconfig=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

mkdir -p $scriptdirSM
mkdir -p $logdirSM
mkdir -p $subimagedirSM
mkdir -p $parsetdirSM

cd ${smdir}
WORKDIR=run${RUN_NUM}
mkdir -p ${WORKDIR}
cd ${WORKDIR}

dependSM=${depend}

if [ $doTaylorSM == true ]; then

    if [ $doSubSM == true ]; then

	modelToTTtemplate=${scriptdirSM}/Simulation-ModelToTaylorTerms-TEMPLATE-${now}.py
	cat > $modelToTTtemplate <<EOF
#!/bin/env python

###############
## AUTOMATICALLY GENERATED - DO NOT EDIT
###############

# A casapy + scipy script to fit a quadratic to the spectra of an input model and create taylor.0 and taylor.1 images

from numpy import *
from scipy import polyfit
import sys

model='${modelimage}'
outputbase='${baseimage}_wINDEX-REPLACEME'
xdim=${npix} / ${nsubxTT}
xmin=(INDEX-REPLACEME % ${nsubxTT}) * xdim
ydim=${npix} / ${nsubyTT}
ymin=(INDEX-REPLACEME / ${nsubxTT}) * ydim

casalog.post('Running with index %d, using xmin=%d and ymin=%d'%(INDEX-REPLACEME,xmin,ymin))

taylor0name='%s/%s.taylor.0'%('${subimagedirSM}',outputbase)
taylor1name='%s/%s.taylor.1'%('${subimagedirSM}',outputbase)
taylor2name='%s/%s.taylor.2'%('${subimagedirSM}',outputbase)

casalog.post('Using %s to create Taylor images called %s/%s.taylor.*'%(model,'${subimagedirSM}',outputbase))

os.system('rm -rf %s'%taylor0name)
os.system('rm -rf %s'%taylor1name)
os.system('rm -rf %s'%taylor2name)

ia.open(model)
shape=ia.shape()
csys=ia.coordsys()
beam=ia.restoringbeam()
spec=csys.findcoordinate(type='spectral')['pixel']
spsize=shape[spec]
haveStokes=(array(csys.names())=='Stokes').any()
if(haveStokes):
    stok=csys.findcoordinate(type='stokes')['pixel']
    nstok=shape[stok]
else:
    nstok=1

outshape=[xdim,ydim,1,1]
outshape[spec]=1
if(haveStokes):
    outshape[stok]=1

casalog.post('Will make images of shape %s'%outshape)

taylor0arr=zeros(outshape)
taylor1arr=zeros(outshape)
taylor2arr=zeros(outshape)

faxis=[csys.referencepixel()['numeric'][spec],csys.referencevalue()['numeric'][spec],csys.increment()['numeric'][spec]]
freq=array(faxis[1] + (range(spsize)-faxis[0])*faxis[2])
xdat=log(freq/faxis[1])

if(haveStokes):
    fullBLC=[xmin,ymin,0,0]
    fullTRC=[xmin+outshape[0]-1,ymin+outshape[1]-1,0,spsize-1]
else:
    fullBLC=[xmin,ymin,0]
    fullTRC=[xmin+outshape[0]-1,ymin+outshape[1]-1,spsize-1]

fullInput = ia.getchunk(blc=fullBLC,trc=fullTRC,dropdeg=True)
casalog.post('Acquired chunk from model of size %d or %fGB'%(fullInput.size,fullInput.size*4./1024**3))

for y in range(outshape[1]):
    for x in range(outshape[0]):

        if( (x+y*outshape[0]) % (outshape[1]*outshape[0]/20) == 0):
            casalog.post('Done %d out of %d spectra, with x=%d and y=%d'%(x+y*outshape[0],outshape[0]*outshape[1],x,y))

        Iref = ia.pixelvalue([x,y,faxis[0]])['value']['value']
        spectrum = fullInput[x,y,:]
        ydat = log(spectrum)

        fit = polyfit(xdat,ydat,2)
        IzeroFit=exp(fit[2])
        alphaFit=fit[1]
        betaFit=fit[0]
        taylor0arr[imloc]=IzeroFit
        taylor1arr[imloc]=alphaFit*IzeroFit
        taylor2arr[imloc]=(betaFit+0.5*alphaFit*(alphaFit-1))*IzeroFit

ia.close()

casalog.post('Done calculating spectra. Now to write the images')

ia.newimagefromshape(outfile=taylor0name,shape=outshape,csys=csys.torecord(),overwrite=true)
ia.open(taylor0name)
ia.putchunk(pixels=taylor0arr)
if ( beam != {} ):
    ia.setrestoringbeam(beam=beam)
ia.close()

ia.newimagefromshape(outfile=taylor1name,shape=outshape,csys=csys.torecord(),overwrite=true)
ia.open(taylor1name)
ia.putchunk(pixels=taylor1arr)
if ( beam != {} ):
    ia.setrestoringbeam(beam=beam)
ia.close()

ia.newimagefromshape(outfile=taylor2name,shape=outshape,csys=csys.torecord(),overwrite=true)
ia.open(taylor2name)
ia.putchunk(pixels=taylor2arr)
if ( beam != {} ):
    ia.setrestoringbeam(beam=beam)
ia.close()

EOF


	runOK=true
	if [ $doFixupSM == true ]; then
	    
	    if [ -e ${failureListSM} ]; then
		INDEX="\`head -\${PBS_ARRAY_INDEX} ${failureListSM} | tail -1\`"
		makeTaylorQsub=${smdir}/${WORKDIR}/makeTaylorTerms-fixup-${now}.qsub
		numFixUp=`wc -l ${failureListSM}`
# Created $fixuplist by something like the following
# grep Exit mkTaylor.o563765.* | grep -v "Code: 0" | sed -e 's/\./ /g' | sed -e 's/:/ /g' | awk '{print $3}' > dudWorkers-taylor.txt
		jobname=mkTaylorFixup
	    else
		echo "Sky Model failure list ${failureListSM} does not exist. Not runnning"
		runOK=false
	    fi
	else
	    INDEX="\${PBS_ARRAY_INDEX}"
	    makeTaylorQsub=${smdir}/${WORKDIR}/makeTaylorTerms-${now}.qsub
	    jobname=mkTaylor
	fi

	cat > $makeTaylorQsub <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=12:00:00
#PBS -l select=1:ncpus=1:mem=${memoryTT}GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N ${jobname}
#PBS -m bea
#PBS -j oe

#########
# AUTOMATICALLY GENERATED - DO NOT EDIT
# TO BE RUN IN BATCH MODE
#########

cd \$PBS_O_WORKDIR

IND=${INDEX}

mkdir -p \${IND}
cd \${IND}

makeTaylor=${scriptdirSM}/Simulation-ModelToTaylorTerms_\${IND}.py
sed -e "s|INDEX-REPLACEME|\${IND}|g" ${modelToTTtemplate} > \${makeTaylor}

output=${logdirSM}/makeTaylorTerms_\${PBS_JOBID}.log
casapy --nologger --log2term -c \${makeTaylor} > \$output
err=\$?
if [ \$err != 0 ]; then
  exit \$err
fi
EOF


	
	if [ $doSubmit == true ] && [ $runOK == true ]; then

	    if [ $doFixupSM == true ]; then
		taylorID=`qsub $dependSM -J 1-$numFixUp $makeTaylorFixQsub`
		echo Submitted fix-up Taylor-term-creation job with ID $taylorID with dependency $depend
	    else
		batchString=`echo $nsubxTT $nsubyTT | awk '{printf "0-%d",$1*$2-1}'`
		taylorID=`qsub $dependSM -J $batchString $makeTaylorQsub`
		echo Submitted Taylor-term-creation job with ID $taylorID with dependency $depend
	    fi

	    dependSM="-W depend=afterok:${taylorID}"

	fi

    fi

####################
# Stitching together

    comScriptBase=${scriptdirSM}/combineTaylorSubs-${now}.py
    comScript=${scriptdirSM}/combineTaylorSubs-\${PBS_JOBID}.py
    
    createFullModel=true
    minWorker=1
    numworkers=551

    cat > $comScriptBase <<EOF
#!/bin/env python

from numpy import *
import os

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
# CASA script to create a full-size continuum model by combining the 455 subcubes

baseDir='${smdir}'
imageDir='${subimagedirSM}'
inputBase='${baseimage}'

imsize=${npix}
spsize=1
stsize=1
outshape=[imsize,imsize,stsize,spsize]

cellsize=${cellsize}
chanw=${chanw}

ra=187.5
dec=-45.
freq=${rfreq}

makeIt=${createFullModel}
minWorker=${minWorker}

for t in range(3):

    suffix='.taylor.%d'%t
    fullimage=baseDir+'/'+inputBase+suffix

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
        input="%s/%s_w%d%s"%(imageDir,inputBase,im,suffix)

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

    comQsub=${smdir}/${WORKDIR}/combineTaylorSubs-${now}.qsub
    cat > $comQsub <<EOF
#!/bin/bash -l
#PBS -W group_list=astronomy116
#PBS -l walltime=12:00:00
#PBS -l select=1:ncpus=1:mem=10GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N combTaylor
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

mv $comScriptBase $comScript

output=${logdirSM}/combineTaylorSubs-\${PBS_JOBID}.log
echo Running casa script to combine subimages into single Taylor term images  > \$output
casapy --nologger --log2term -c $comScript >> \$output
exit \$?
EOF
    
    if [ $doSubmit == true ]; then

	comID=`qsub ${dependSM} $comQsub`
	echo Submitted job $comID to combine Taylor term subimages into final images
	if [ "$dependSM" == "" ]; then
	    dependSM="-W depend=afterok:${comID}"
	else
	    dependSM="${dependSM}:${comID}"
	fi
    fi

fi


####################
# Smooth the models

if [ $doSmoothSM == true ]; then

    smoothScriptBase=${scriptdirSM}/smoothModels-${now}.py
    smoothScript=${scriptdirSM}/smoothModels-\${PBS_JOBID}.py
    cat > $smoothScriptBase <<EOF
#!/bin/env python

from numpy import *
import os

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
# CASA script to create a full-size continuum model by combining the 455 subcubes

for t in range(3):

    modelIm='${modelimage}.taylor.%d'%t
    smoothIm='${baseimage}-smooth.taylor.%d'%t

    ia.open(modelIm)
    ia.convolve2d(outfile=smoothIm,major='${smoothBmaj}arcsec',minor='${smoothBmin}arcsec',pa='${smoothBpa}deg')
    ia.close()
    ia.open(smoothIm)
    ia.setrestoringbeam(major='${smoothBmaj}arcsec',minor='${smoothBmin}arcsec',pa='${smoothBpa}deg')
    ia.close()
EOF

    smoothQsub=${smdir}/${WORKDIR}/smoothModels-${now}.qsub
    cat > $smoothQsub <<EOF
#!/bin/bash -l
#PBS -q debugq
#PBS -W group_list=astronomy116
#PBS -l walltime=1:00:00
#PBS -l select=1:ncpus=1:mem=10GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N smoothTaylor
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR

mv $smoothScriptBase $smoothScript

output=${logdirSM}/smoothModels-\${PBS_JOBID}.log
echo Running casa script to smooth  Taylor term images  > \$output
casapy --nologger --log2term -c $smoothScript >> \$output
exit \$?
EOF
    
    if [ $doSubmit == true ]; then

	smoothID=`qsub ${dependSM} $smoothQsub`
	echo Submitted job $smoothID to smooth Taylor term images
	if [ "$dependSM" == "" ]; then
	    dependSM="-W depend=afterok:${smoothID}"
	else
	    dependSM="${dependSM}:${smoothID}"
	fi

    fi

fi


#####################
# Find & fit sources

if [ $doSF_SM == true ]; then

    cduchampParsetBase=${parsetdirSM}/cduchamp-smooth-${now}.in
    cduchampParset=${parsetdirSM}/cduchamp-smooth-\${PBS_JOBID}.in

    cat > $cduchampParsetBase <<EOF
Cduchamp.imageFile = ${baseimage}-smooth.taylor.0
Cduchamp.threshold = ${SFthresh}
Cduchamp.flagGrowth = ${SFflagGrowth}
Cduchamp.growthThreshold = ${SFgrowthThresh}
Cduchamp.nsubx = ${SFnsubx}
Cduchamp.nsuby = ${SFnsuby}
Cduchamp.doFit = true
Cduchamp.fitJustDetection = true
Cduchamp.Fitter.useNoise = false
Cduchamp.Fitter.noiseLevel = 1.e-3
Cduchamp.Fitter.numSubThresholds = 1000
Cduchamp.findSpectralIndex = true
EOF

    cduchampQsub=${smdir}/${WORKDIR}/cduchamp-smooth-${now}.qsub
    cat > $cduchampQsub <<EOF
#!/bin/bash -l
#PBS -q debugq
#PBS -W group_list=astronomy116
#PBS -l walltime=1:00:00
#PBS -l select=${SFnNodes}:ncpus=12:mem=23GB:mpiprocs=12
#PBS -M matthew.whiting@csiro.au
#PBS -N cduchampTaylor
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR
export AIPSPATH=${AIPSPATH}

mv $cduchampParsetBase $cduchampParset

output=${logdirSM}/cduchamp-smooth-\${PBS_JOBID}.log
mpirun $cduchamp -inputs $cduchampParset > \$output

exit \$?

EOF

    if [ $doSubmit == true ]; then

	cduchampID=`qsub ${dependSM} $cduchampQsub`
	echo Submitting Cduchamp job with ID $cduchampID
	if [ "$dependSM" == "" ]; then
	    dependSM="-W depend=afterok:${cduchampID}"
	else
	    dependSM="${dependSM}:${cduchampID}"
	fi

    fi

fi

#####################
# Find & fit sources

if [ $doComparisonSM == true ]; then

    # make a comparison image - single channel only
    # then use imagecalc in casacore to find the difference between the two

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

    modelcompParsetBase=${parsetdirSM}/modelcomp-${now}.in
    modelcompParset=${parsetdirSM}/modelcomp-\${PBS_JOBID}.in
    cat > $modelcompParsetBase <<EOF
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
createFITS.sourcelist       = duchamp-fitResults.txt
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
EOF

    modelcompQsub=${smdir}/${WORKDIR}/modelComparison-${now}.qsub
    cat > $modelcompQsub <<EOF
#!/bin/bash -l
#PBS -q debugq
#PBS -W group_list=astronomy116
#PBS -l walltime=1:00:00
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
#PBS -M matthew.whiting@csiro.au
#PBS -N modelComp
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \$PBS_O_WORKDIR
export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=${AIPSPATH}
export CASACOREDIR=\${ASKAP_ROOT}/3rdParty/casacore/casacore-1.3.0
createFITS=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/install/bin/createFITS.sh
imagecalc=\${CASACOREDIR}/install/bin/imagecalc

mv $modelcompParsetBase $modelcompParset

output=${logdirSM}/modelcomp-\${PBS_JOBID}.log
mpirun \$createFITS -inputs $modelcompParset > \$output
err=\$?
if [ \$err -ne 0 ]; then
    exit \$?
fi

. \${CASACOREDIR}/init_package_env.sh
\${imagecalc} in="'${baseimage}-smooth.taylor.0' - '${modelcompImage}'" out='${modelcompDiff}'
err=\$?
exit \$err

EOF

    if [ $doSubmit == true ]; then
	
	modelcompID=`qsub $dependSM $modelcompQsub`
	echo Submitting Model Comparison job with ID $modelcompID

    fi

fi

cd ${BASEDIR}
