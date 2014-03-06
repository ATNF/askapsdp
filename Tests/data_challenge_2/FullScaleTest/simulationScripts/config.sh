#!/bin/bash -l

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

doCorrupt=true
randomgainsparset=${parsetdir}/randomgains.in
doNoise=true
Tsys=50
nchan=16416
freqChanZeroMHz=1050
chanw=-18.5185185e3
rchan=0
pol="XX YY"

inttime=5s

##################################
# For 1934 calibration observation
#

firstPointingID=0
lastPointingID=8

msbaseCal=calibrator_J1934m638_${inttime}_${now}

ra1934=294.854275
ra1934str=19h39m25.036
dec1934=-63.712675
dec1934str=-63.42.45.63
direction1934="[${ra1934str}, ${dec1934str}, J2000]"

calHArange=(
"-0.5h,-0.4h" 
"-0.4h,-0.3h" 
"-0.3h,-0.2h" 
"-0.2h,-0.1h" 
"-0.1h,0.0h" 
"0.0h,0.1h" 
"0.1h,0.2h" 
"0.2h,0.3h" 
"0.3h,0.4h"
)

#Gridding parameters
nw=201
os=8


###############################
# For science field observation
#

#model=SKADS
model=BETAtestfield

if [ $model == "SKADS" ]; then 

    msbaseSci=sciencefield_SKADS_${inttime}_${now}
    feeds=${askapconfig}/ASKAP9feeds.in

    NGROUPS_CSIM=16
    NWORKERS_CSIM=171
    NCPU_CSIM=`echo $NWORKERS_CSIM | awk '{print $1+1}'`
    NPPN_CSIM=20
    chanPerMSchunk=6
    
    doFlatSpectrum=false
    chunkdir=/scratch/askap/whi550/Simulations/BETA/InputModels/Images/Chunks
#slicedir=../ModelImages/Slices
    slicedir=/scratch/askap/whi550/Simulations/BETA/InputModels/Images/Slices
    baseimage=DCmodel_cont_dec45_1050_smallBETA
    modelimage=${chunkdir}/${baseimage}
    writeByNode=true
    
# Whether to slice up the model prior to simulating
    doSlice=true
#doSlice=false
    npixModel=2304
    nsubxCR=18
    nsubyCR=11
    SLICERWIDTH=100
    SLICERNPPN=20

    baseDirection="[12h30m00.000, -45.00.00, J2000]"

else

    msbaseSci=sciencefield_BETAtestfield_${inttime}_${now}

    feeds=BETAtestfield_feeds.in
    cat > $feeds <<EOF
# A feeds parset that tries to replicate the beams used in the
# commissioning observations with BETA in January/February 2014.
# Offsets taken from https://pm.antf.csiro.au/askap/projects/seic/wiki/CommissioningRunJan2014_ 
# (from the 1 Feb entry), with the RA offset sign reversed.

feeds.spacing        =       1deg

feeds.names     = [feed0, feed1, feed2, feed3, feed4, feed5, feed6, feed7, feed8]

feeds.feed0          =       [0,0]
feeds.feed1          =       [-0.572425, 0.947258]
feeds.feed2          =       [-1.14485, 1.89452]
feeds.feed3          =       [0.572425, -0.947258]
feeds.feed4          =       [-1.23347, -0.0987957]
feeds.feed5          =       [-1.8059, 0.848462]
feeds.feed6          =       [0.661046, 1.04605]
feeds.feed7          =       [0.0886209, 1.99331]
feeds.feed8          =       [1.23347, 0.0987957]
EOF
    
    NGROUPS_CSIM=16
    NWORKERS_CSIM=171
    NCPU_CSIM=`echo $NWORKERS_CSIM | awk '{print $1+1}'`
    NPPN_CSIM=20
    chanPerMSchunk=6
    
    doFlatSpectrum=true
    slicedir=/scratch/askap/whi550/Simulations/DC2/FullBandwidth/InputModels/Images
    baseimage=BETAtestfield_SUMSSmodel_dec79_1050_flat
    writeByNode=false
    
# Whether to slice up the model prior to simulating
    doSlice=false

    baseDirection="[15h56m58.870,-79.14.04.28, J2000]"
    

fi

spwbaseSci=${parsetdir}/spws_sciencefield

observationLengthHours=12
dur=`echo $observationLengthHours | awk '{print $1/2.}'`
