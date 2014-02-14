#!/bin/bash -l 

slicer=${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/makeModelSlice.sh
createFITS=${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh
rndgains=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/randomgains.sh
csim=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/csimulator.sh
ccal=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh
cim=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh
mssplit=${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/mssplit.sh
askapconfig=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

now=`date +%F-%H%M`

doScienceField=true
modelImage=scienceModel

npix=2048
cellsize=10.
delt=`echo $cellsize | awk '{print $1/3600.}'`
nstokes=1
stokesZero=0
dstokes=1
ra=187.5
dec=-45
rpix=`echo $npix | awk '{print $1/2}'`
freqChanZeroMHz=1421
rfreq=`echo ${freqChanZeroMHz} | awk '{printf "%8.6e",$1*1.e6}'`
rchan=0
nchan=1
chanw=-18.5185185e3
pol="XX YY"
nurefMHz=`echo ${rfreq} ${CHAN} ${rchan} ${chanw} | awk '{printf "%13.8f",($1+($2-$3)*$4)/1.e6}'`
basefreq=`echo $nchan $rchan $rfreq $chanw | awk '{printf "%8.6e",$3 + $4*($2+$1/2)}'`

#Gridding parameters
nw=129
os=8

feedparset="${askapconfig}/ASKAP9feeds.in"

doCorrupt=true
randomgainsparset=randomgains.in
doCal=true

msbase=calTest
baseDirection="[12h30m00.000, -45.00.00, J2000]"
dirlist=(
"[12h35m33.639, -44.00.00, J2000]" 
"[12h35m39.411, -45.00.00, J2000]"
"[12h35m45.494, -46.00.00, J2000]"
"[12h30m00.000, -44.00.00, J2000]"
"[12h30m00.000, -45.00.00, J2000]"
"[12h30m00.000, -46.00.00, J2000]"
"[12h24m26.361, -44.00.00, J2000]"
"[12h24m20.589, -45.00.00, J2000]"
"[12h24m14.506, -46.00.00, J2000]"
)

radian=`echo 1 | awk '{printf "%16.14f",$1*180./atan2(0.,-0.)}'`
raOff=(-1 -1 -1 0 0 0 1 1 1)
decOff=(-1 0 1 -1 0 1 -1 0 1)

feedlist=(6 7 8 3 4 5 0 1 2)

