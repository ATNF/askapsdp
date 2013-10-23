#!/bin/bash -l

# Need to define the following beforehand:
# sourcelist, databaseCR, freqChanZeroMHz, useGaussianComponents,
# baseimage, msbase

##############################
## Directories

# Model
logdirCR=${crdir}/Logs
parsetdirCR=${crdir}/Parsets
imagedir=${crdir}/Images
chunkdir=${imagedir}/Chunks
slicedir=${imagedir}/Slices

# Sky Model
scriptdirSM=${smdir}/Scripts
logdirSM=${smdir}/Logs
subimagedirSM=${smdir}/Subimages
parsetdirSM=${smdir}/Parsets

# Visibilities
msdir=${visdir}/MS
parsetdirVis=${visdir}/Parsets
logdirVis=${visdir}/Logs

##############################
## Switches

# Model
doFlatSpectrum=true
doCreateCR=true
doSliceCR=true
if [ $doFlatSpectrum == "true" ]; then
    doSliceCR=false
fi

# Sky Model
doSmoothSM=true
doSF_SM=true
doComparisonSM=true

# Visibilities
doCsim=true
doVisCleanup=false
failureListVis="EmptyFile"
#failureListVis="/scratch/astronomy554/whi550/DataChallenge/Simulation/failure-vis-at200120318-2nd.txt"
doMergeVis=true
doClobberMergedVis=true
doMergeStage1=true
doMergeStage2=true

doNoise=true
varNoise=false
doCorrupt=false
doAntennaBased=false

###########################################
# Model creation definitions

npix=2245
# 2245 pix @ 9.1234"/pix gives area covered by 2048 @ 10"/pix
rpix=`echo $npix | awk '{print $1/2}'`
cellsize=9.1234
delt=`echo $cellsize | awk '{print $1/3600.}'`
ra=187.5
dec=-45.0
raCat=0.
decCat=0.
decSuffix=`echo $dec | awk '{printf "dec%02d",-$1}'`
baseimage="${baseimage}_${decSuffix}"
msbase="${msbase}_${decSuffix}"

nchan=8208
rchan=0
chanw=-18.5185185e3
rfreq=`echo ${freqChanZeroMHz} $nchan $chanw | awk '{printf "%8.6e",$1*1.e6+$2*$3/2.}'`

baseimage="${baseimage}_${freqChanZeroMHz}_smallBETA"

nstokes=1
rstokes=0
stokesZero=0
dstokes=0

nsubxCR=5
nsubyCR=8
workersPerNodeCR=1

writeByNode=true
modelimage=${imagedir}/${baseimage}
createTT_CR=true
if [ $writeByNode == "true" ]; then
    doSliceCR=false
    modelimage=${chunkdir}/${baseimage}
fi
slicebase=${slicedir}/${baseimage}_chunk

###########################################
# Make the visibilities

array=BETAXYZ.in
nfeeds=9
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

chanPerMSchunk=48
numMSchunks=171
msPerStage1job=9

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

if [ $useGaussianComponents != "true" ]; then
    msbase="${msbase}_discs"
fi
if [ $databaseCR == "POSSUM" ]; then
    msbase="${msbase}_cont"
elif [ $databaseCR == "POSSUMHI" ]; then
    msbase="${msbase}_HI"
fi
msbase="${msbase}_smallBETA"

###########################################
# Make the sky model image

smoothBmaj=47
smoothBmin=33
smoothBpa=144.25

SFthresh=1.e-3
SFflagGrowth=true
SFgrowthThresh=5.e-4
SFnsubx=7
SFnsuby=5
SFnNodes=`echo $SFnsubx $SFnsuby | awk '{print int(($1*$2-0.001)/12.)+1}'`
