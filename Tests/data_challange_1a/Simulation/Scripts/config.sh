#!/bin/bash -l

##############################
# TOP LEVEL
##############################

export AIPSPATH=${ASKAP_ROOT}/Code/Base/accessors/current

crdir=${BASEDIR}/InputModels
visdir=${BASEDIR}/Visibilities
smdir=${BASEDIR}/SkyModel
catdir=${BASEDIR}/InputCatalogue

depend=""
now=`date +%F-%H%M`
begin="<""<""EOF"
end="EOF"

queueName=routequeue

##############################
# MODEL CREATION
##############################

doCreateCR=false
doCombineCR=false
#doSliceCR=${doCreateCR}
doSliceCR=true
createFullModelCR=false
minWorkerCR=1

logdirCR=${crdir}/Logs
parsetdirCR=${crdir}/Parsets
scriptdirCR=${crdir}/Scripts
imagedir=${crdir}/Images
slicedir=${imagedir}/Slices

baseimage=DCmodelfull-cont
modelimage=${imagedir}/${baseimage}
slicebase=${slicedir}/${baseimage}-chunk

sourcelist=master_possum_catalogue_trim10x10deg.dat

npix=3560
rpix=1780
nchan=16416
chanPerMSchunk=19
numMSchunks=864
if [ `echo $chanPerMSchunk  $numMSchunks | awk '{print $1*$2}'` != $nchan ]; then
    echo ERROR - nchan = $nchan, but chanPerMShunk = $chanPerMSchunk and numMSchunks = $numMSchunks
    echo Not running script.
    doSubmit=false
fi
rchan=1
cellsize=9.1234
delt=`echo $cellsize | awk '{print $1/3600.}'`
chanw=18.5185185e3
rfreq=1.421e9
nstokes=1

nsubxCR=10
nsubyCR=8
workersPerNodeCR=1

createTT_CR=true


##############################
# Sky Model creation
##############################

scriptdirSM=${smdir}/Scripts
logdirSM=${smdir}/Logs
subimagedirSM=${smdir}/Subimages
parsetdirSM=${smdir}/Parsets

doTaylorSM=false
doSubSM=true
doFixupSM=false
failureListSM="EmptyFile"
doSmoothSM=true
doSF_SM=true
doComparisonSM=true

memoryTT=23
nsubxTT=10
nsubyTT=10

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

doCsim=false
doVisCleanup=true
#failureListVis="EmptyFile"
failureListVis="/scratch/astronomy116/whi550/DataChallenge/Simulation/failure-vis-at200120318-2nd.txt"
doMergeVis=true
doClobberMergedVis=true
doMergeStage1=true
doMergeStage2=true

msdir=${visdir}/MS
msbase=${msdir}/DCvis_chunk
msStage1base=${msdir}/DCvis_stage1
finalMS=${msdir}/DC1a.ms
parsetdirVis=${visdir}/Parsets
logdirVis=${visdir}/Logs

msPerStage1job=36
numStage1jobs=`echo $numMSchunks $msPerStage1job | awk '{print int($1/$2)}'`
if [ `echo $msPerStage1job $numStage1jobs | awk '{print $1*$2}'` != $numMSchunks ]; then
    echo ERROR - numMSchunks = $numMSchunks and msPerStage1job = $msPerStage1job but numStage1jobs = $numStage1jobs
    echo Not running script.
    doSubmit=false
fi

array=BETA15.in
feeds=ASKAP36feeds.in
inttime=30s
dur=6
chanWidth_kHz=18.31055
doNoise=true
tsys=50.

doSnapshot=true
wtol=3000
gridder=AWProject
if [ $doSnapshot == true ]; then
    wmax=3000
else
    wmax=30000
fi
nw=17
os=8
maxsup=8192
pad=1.

