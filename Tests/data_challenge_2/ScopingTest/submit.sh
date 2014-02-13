#!/bin/bash -l

scriptdir=`pwd`/scripts

runIt=true

if [ "${ASKAP_ROOT}" == "" ]; then
    echo "You have not set ASKAP_ROOT! Exiting."
    runIt=false
fi

if [ "${AIPSPATH}" == "" ]; then
    echo "You have not set AIPSPATH! Exiting."
    runIt=false
fi

if [ $runIt == true ]; then    

    . ${scriptdir}/config.sh

    #POINTING=2
    #MAXPOINT=2

    POINTING=0
    MAXPOINT=8

    workdir="run_${now}"
    mkdir -p $workdir
    cd $workdir
    mkdir -p parsets
    mkdir -p logs

    if [ $doCorrupt == true ]; then
	$rndgains -f 9 -a 6 -p 2 $randomgainsparset
    fi

    while [ $POINTING -le $MAXPOINT ]; do

	doCorrupt=false
	doCal=false
	depend=""
	. ${scriptdir}/runCsimulator.sh
	depend="-Wdepend=afterok:${latestID}"
	. ${scriptdir}/runImaging.sh
	doCorrupt=true
	depend=""
	. ${scriptdir}/runCsimulator.sh
	depend="-Wdepend=afterok:${latestID}"
	. ${scriptdir}/runImaging.sh
	doCal=true
	. ${scriptdir}/runCcalibrator.sh
	depend="${depend}:${latestID}"
	. ${scriptdir}/runImaging.sh

	POINTING=`expr $POINTING + 1`

    done

    cd ..

fi
