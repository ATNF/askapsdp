#!/bin/bash -l

##############################
# TOP LEVEL
##############################

if [ ${ASKAP_ROOT} == "" ]; then
    echo "Your \$ASKAP_ROOT variable is not set. Not running script!"
    exit 1
fi

export AIPSPATH=${ASKAP_ROOT}/Code/Base/accessors/current

crdir=${BASEDIR}/InputModels
visdir=${BASEDIR}/Visibilities
smdir=${BASEDIR}/SkyModel
catdir=${BASEDIR}/InputCatalogue

depend=""
now=`date +%F-%H%M`

queueName=routequeue

##############################
# FUNDADMENTAL
##############################

sourcelist=master_possum_catalogue_trim10x10deg.dat

databaseCR=POSSUM
#databaseCR=POSSUMHI

doSmallBETA=false

#freqChanZeroMHz=1421
freqChanZeroMHz=1050

baseimage="DCmodel"

array=BETAXYZ.in
if [ $doSmallBETA == true ]; then
    nfeeds=9
else
    nfeeds=36
fi

msbase=DCvis

##############################
# MODEL CREATION
##############################

doCreateCR=true
doSliceCR=true

logdirCR=${crdir}/Logs
parsetdirCR=${crdir}/Parsets
imagedir=${crdir}/Images
chunkdir=${imagedir}/Chunks
slicedir=${imagedir}/Slices

if [ $databaseCR == "POSSUM" ]; then
    listtypeCR=continuum
    baseimage="${baseimage}_cont"
elif [ $databaseCR == "POSSUMHI" ]; then
    listtypeCR=spectralline
    baseimage="${baseimage}_HI"
fi

if [ $doSmallBETA == true ]; then
    npix=2245
# 2245 pix @ 9.1234"/pix gives area covered by 2048 @ 10"/pix
else
    npix=3560
fi
#rpix=1780
rpix=`echo $npix | awk '{print $1/2}'`
cellsize=9.1234
delt=`echo $cellsize | awk '{print $1/3600.}'`
ra=187.5
dec=-45.0
raCat=0.
decCat=0.

baseimage="${baseimage}_${freqChanZeroMHz}"

nchan=16416
rchan=0
chanw=-18.5185185e3
rfreq=`echo ${freqChanZeroMHz} | awk '{printf "%8.6e",$1*1.e6}'`
if [ $doSmallBETA == true ]; then
    nchan=`echo $nchan | awk '{print $1/2.}'`
    rfreq=`echo $nchan $rfreq $chanw | awk '{print $2 + $3*$1/2.}'`
    baseimage="${baseimage}_smallBETA"
fi
basefreq=`echo $nchan $rchan $rfreq $chanw | awk '{printf "%8.6e",$3 + $4*($2+$1/2)}'`

if [ $doSmallBETA == true ]; then
    chanPerMSchunk=48
    numMSchunks=171
else
    chanPerMSchunk=19
    numMSchunks=864
fi

if [ `echo $chanPerMSchunk  $numMSchunks | awk '{print $1*$2}'` != $nchan ]; then
    echo ERROR - nchan = $nchan, but chanPerMShunk = $chanPerMSchunk and numMSchunks = $numMSchunks
    echo Not running script.
    doSubmit=false
fi

nstokes=1
rstokes=0
stokesZero=0
dstokes=0

if [ $nstokes -gt 1 ]; then
    baseimage=${baseimage}_fullstokes
fi

if [ $doSmallBETA == true ]; then
    nsubxCR=2
    nsubyCR=4
else
    nsubxCR=5
    nsubyCR=8
fi
workersPerNodeCR=1
writeByNode=true
if [ $writeByNode == "true" ]; then
    doSliceCR=false
fi

createTT_CR=true

modelimage=${imagedir}/${baseimage}
if [ $writeByNode == "true" ]; then
    modelimage=${chunkdir}/${baseimage}
fi
slicebase=${slicedir}/${baseimage}_chunk


##############################
# Sky Model creation
##############################

scriptdirSM=${smdir}/Scripts
logdirSM=${smdir}/Logs
subimagedirSM=${smdir}/Subimages
parsetdirSM=${smdir}/Parsets

doSmoothSM=true
doSF_SM=true
doComparisonSM=true

smoothBmaj=47
smoothBmin=33
smoothBpa=144.25

SFthresh=1.e-3
SFflagGrowth=true
SFgrowthThresh=5.e-4
SFnsubx=7
SFnsuby=5
SFnNodes=`echo $SFnsubx $SFnsuby | awk '{print int(($1*$2-0.001)/12.)+1}'`


##############################
# Visibilities
##############################

doCsim=true
doVisCleanup=false
failureListVis="EmptyFile"
#failureListVis="/scratch/astronomy554/whi550/DataChallenge/Simulation/failure-vis-at200120318-2nd.txt"
doMergeVis=true
doClobberMergedVis=true
doMergeStage1=true
doMergeStage2=true

feeds=ASKAP${nfeeds}feeds.in
inttime=30s
dur=6

pol="XX YY"
npol=2
polName="${npol}pol"

doNoise=true
varNoise=false
noiseSlope=0.2258
noiseIntercept=-188.71
freqTsys50=`echo $noiseSlope $noiseIntercept | awk '{print (50.-$2)/$1}'`
tsys=50.
if [ $doNoise == true ]; then
    if [ $varNoise == true ]; then
	noiseName="noiseVariable"
    else
	noiseName="noise${tsys}K"
    fi
else
    noiseName="noNoise"
fi

doCorrupt=false
doAntennaBased=false
calibparset=rndgains.in
if [ $doCorrupt == true ]; then
    if [ $doAntennaBased == true ]; then
	antbasedname="antbased"
	randomgainsArgs="-a 6 -p ${npol}"
    else
	antbasedname="feedbased"
	randomgainsArgs="-f ${nfeeds} -a 6 -p ${npol}"
    fi
    corruptName="corrupt_${antbasedname}"
else
    corruptName="noCorrupt"
fi

msdir=${visdir}/MS
if [ $databaseCR == "POSSUM" ]; then
    msbase="${msbase}_cont"
elif [ $databaseCR == "POSSUMHI" ]; then
    msbase="${msbase}_HI"
fi
msbase="${msbase}_${freqChanZeroGHz}_${polName}_${nfeeds}feeds_${noiseName}_${corruptName}"

if [ $doSmallBETA == true ]; then
    msbase="${msbase}_smallBETA"
fi

msChunk=${msdir}/${msbase}_chunk
msStage1=${msdir}/${msbase}_stage1
finalMS=${msdir}/${msbase}.ms

parsetdirVis=${visdir}/Parsets
logdirVis=${visdir}/Logs

# How many of the initial MSs to combine into each of the stage1 MSs
if [ $doSmallBETA == true ]; then
    msPerStage1job=9
else
    msPerStage1job=36
fi
numStage1jobs=`echo $numMSchunks $msPerStage1job | awk '{print int($1/$2)}'`
if [ `echo $msPerStage1job $numStage1jobs | awk '{print $1*$2}'` != $numMSchunks ]; then
    echo ERROR - numMSchunks = $numMSchunks and msPerStage1job = $msPerStage1job but numStage1jobs = $numStage1jobs
    echo Not running script.
    doSubmit=false
fi

doSnapshot=true
wtol=1000
gridder=AWProject
if [ $doSnapshot == true ]; then
    wmax=1000
    maxsup=512
else
    wmax=15000
    maxsup=8192
fi
nw=129
os=4
pad=1.

