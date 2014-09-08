#!/usr/bin/bash

# General

slicer=${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/makeModelSlice.sh
createFITS=${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh
rndgains=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/randomgains.sh
csim=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/csimulator.sh
ccal=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh
cim=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh
mssplit=${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/mssplit.sh
msmerge=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh
askapconfig=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

now=`date +%F-%H%M`

parsetdir=parsets
logdir=logs
msdir=MS
chunkdir=Images/Chunks
slicedir=Images/Slices

doCreateModel=true
# Whether to slice up the model prior to simulating - set to false
# if we've already done this
doSlice=true

doCorrupt=true
randomgainsparset=${parsetdir}/randomgains.in
doNoise=true
Tsys=50

antennaConfig=ADE12
nfeeds=36
nant=12

pol="XX XY YX YY"

inttime=5s

##################################
# For 1934 calibration observation
#

firstPointingID=0
lastPointingID=`expr ${nfeeds} - 1`

msbaseCal=calibrator_J1934m638_${inttime}_${now}

ra1934=294.854275
ra1934str=19h39m25.036
dec1934=-63.712675
dec1934str=-63.42.45.63
direction1934="[${ra1934str}, ${dec1934str}, J2000]"

. ${simScripts}/makeCalHArange.sh

#Gridding parameters
nw=201
os=8


###############################
# For science field observation
#

msbaseSci=sciencefield_SKADS_${inttime}_${now}
feeds=${askapconfig}/ASKAP${nfeeds}feeds.in
antennaParset=${askapconfig}/${antennaConfig}.in

NGROUPS_CSIM=27
NWORKERS_CSIM=152
NCPU_CSIM=`echo $NWORKERS_CSIM | awk '{print $1+1}'`
NPPN_CSIM=20
chanPerMSchunk=4

catdir=/scratch/askap/whi550/Simulations/BETA/InputCatalogue
sourcelist=master_possum_catalogue_trim10x10deg.dat

doFlatSpectrum=false
baseimage=ASKAP12_ES_SKADS_model
modelimage=${chunkdir}/${baseimage}
writeByNode=true
createTT_CR=true

npixModel=4096
nsubxCR=7
nsubyCR=12
CREATORTASKS=`echo $nsubxCR $nsubyCR | awk '{print $1*$2+1}'`
CREATORWORKERPERNODE=1
CREATORNODES=`echo $CREATORTASKS ${CREATORWORKERPERNODE} | awk '{print int($1/$2)}'`
SLICERWIDTH=100
SLICERNPPN=1

databaseCR=POSSUM
#databaseCR=POSSUMHI
posType=deg
PAunits=rad
useGaussianComponents=true
if [ $databaseCR == "POSSUM" ]; then
    listtypeCR=continuum
    baseimage="${baseimage}_cont"
elif [ $databaseCR == "POSSUMHI" ]; then
    listtypeCR=spectralline
    baseimage="${baseimage}_HI"
fi

# Size and pixel scale of spatial axes
npix=4096
rpix=`echo $npix | awk '{print $1/2}'`
cellsize=6
delt=`echo $cellsize | awk '{print $1/3600.}'`

# Central position for the input model
ra=187.5
dec=-45.0
# And how that is translated for the csimulator jobs
raStringVis="12h30m00.000"
decStringVis="-45.00.00"
baseDirection="[${raStringVis}, ${decStringVis}, J2000]"

# Does the catalogue need precessing? If so, central position of catalogue.
WCSsources=true
raCat=0.
decCat=0.

# Spectral axis - full spectral range & resolution
freqChanZeroMHz=1421
nchan=16416
rchan=0
chanw=-18.5185185e3
rfreq=`echo ${freqChanZeroMHz} | awk '{printf "%8.6e",$1*1.e6}'`
basefreq=`echo $nchan $rchan $rfreq $chanw | awk '{printf "%8.6e",$3 + $4*($2+$1/2)}'`

# Polarisation axis - use full stokes for these models
nstokes=4
rstokes=0
stokesZero=0
dstokes=1


spwbaseSci=${parsetdir}/spws_sciencefield

observationLengthHours=12
# duration for csimulator parset - have observation evenly split over
# transit, so give hour angle start/stop times
dur=`echo $observationLengthHours | awk '{print $1/2.}'`
