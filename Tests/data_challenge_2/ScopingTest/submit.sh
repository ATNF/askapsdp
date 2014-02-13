#!/bin/bash -l

scriptdir=`pwd`

. ./config.sh

#POINTING=2
#MAXPOINT=2

POINTING=0
MAXPOINT=8

workdir="run_${now}"
mkdir -p $workdir
cd $workdir

if [ $doCorrupt == true ]; then
    $rndgains -f 9 -a 6 -p 2 $randomgainsparset
fi

while [ $POINTING -le $MAXPOINT ]; do

    doCorrupt=false
    doCal=false
    depend=""
    . ./runCsimulator.sh
    depend="-Wdepend=afterok:${latestID}"
    . ./runImaging.sh
    doCorrupt=true
    depend=""
    . ./runCsimulator.sh
    depend="-Wdepend=afterok:${latestID}"
    . ./runImaging.sh
    doCal=true
    . ./runCcalibrator.sh
    depend="${depend}:${latestID}"
    . ./runImaging.sh

    POINTING=`expr $POINTING + 1`

done

cd ..

