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

npix=3560
rpix=`echo $npix | awk '{print $1/2}'`
cellsize=9.1234
delt=`echo $cellsize | awk '{print $1/3600.}'`
ra=187.5
dec=-45.0
decStringVis="-45.00.00"
raCat=0.
decCat=0.
decSuffix=`echo $dec | awk '{printf "dec%02d",-$1}'`
baseimage="${baseimage}_${decSuffix}"
msbase="${msbase}_${decSuffix}"

baseimage="${baseimage}_${freqChanZeroMHz}"

nchan=16416
rchan=0
chanw=-18.5185185e3
rfreq=`echo ${freqChanZeroMHz} | awk '{printf "%8.6e",$1*1.e6}'`

nstokes=1
rstokes=0
stokesZero=0
dstokes=0

nsubxCR=16
nsubyCR=16
CREATORWIDTH=`echo $nsubxCR $nsubyCR | awk '{print $1*$2+1}'`
CREATORPPN=20

writeByNode=true
modelimage=${imagedir}/${baseimage}
createTT_CR=true
if [ $writeByNode == "true" ]; then
    modelimage=${chunkdir}/${baseimage}
fi
slicebase=${slicedir}/${baseimage}_slice
SLICERWIDTH=100
SLICERNPPN=20

###########################################
# Make the visibilities

csimSelect="#PBS -l select=1:ncpus=1:mem=20GB:mpiprocs=1"

array=BETAXYZ.in
nfeeds=36
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

chanPerMSchunk=19
numMSchunks=864
msPerStage1job=36

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
doFreqDep=false

if [ $useGaussianComponents != "true" ]; then
    msbase="${msbase}_discs"
fi
if [ $databaseCR == "POSSUM" ]; then
    msbase="${msbase}_cont"
elif [ $databaseCR == "POSSUMHI" ]; then
    msbase="${msbase}_HI"
fi

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
