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

msbaseCal=calibrator_J1934m638_${now}

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

msbaseSci=sciencefield_SKADS_${now}

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
#doSlice=true
doSlice=false
nsubxCR=18
nsubyCR=11
SLICERWIDTH=100
SLICERNPPN=20

baseDirection="[12h30m00.000, -45.00.00, J2000]"

spwbaseSci=${parsetdir}/spws_sciencefield

observationLengthHours=12
dur=`echo $observationLengthHours | awk '{print $1/2.}'`
