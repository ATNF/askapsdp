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

    allIDs="--dependency=afterok"

    while [ $POINTING -le $MAXPOINT ]; do

	doCorrupt=false
	doCal=false
	depend=""
	. ${scriptdir}/runCsimulator.sh
	depend="--dependency=afterok:${latestID}"
	allIDs="${allIDs}:${latestID}"
	. ${scriptdir}/runImaging.sh
	allIDs="${allIDs}:${latestID}"
	doCorrupt=true
	depend=""
	. ${scriptdir}/runCsimulator.sh
	depend="--dependency=afterok:${latestID}"
	allIDs="${allIDs}:${latestID}"
	. ${scriptdir}/runImaging.sh
	allIDs="${allIDs}:${latestID}"
	doCal=true
	. ${scriptdir}/runCcalibrator.sh
	depend="${depend}:${latestID}"
	allIDs="${allIDs}:${latestID}"
	. ${scriptdir}/runImaging.sh
	allIDs="${allIDs}:${latestID}"

	POINTING=`expr $POINTING + 1`

    done

    if [ $doScienceField == true ]; then

	doTrim=false

	depend=${allIDs}
	. ${scriptdir}/combineCalResults.sh
	allIDs="${allIDs}:${latestID}"

	depend=""
	. ${scriptdir}/makeScienceModel.sh
	modelID=${latestID}

	doCorrupt=false
	doCal=false
	depend="--dependency=afterok:${modelID}"
	. ${scriptdir}/runCsimulatorScienceField.sh
	depend="${allIDs}:${modelID}:${latestID}"
	. ${scriptdir}/runImagingScienceField.sh
	. ${scriptdir}/runImagingScienceFieldTrimmed.sh

	doCorrupt=true
	depend="--dependency=afterok:${modelID}"
	. ${scriptdir}/runCsimulatorScienceField.sh
	depend="${allIDs}:${modelID}:${latestID}"
	. ${scriptdir}/runImagingScienceField.sh
	. ${scriptdir}/runImagingScienceFieldTrimmed.sh

	doCal=true
	. ${scriptdir}/runImagingScienceField.sh
	. ${scriptdir}/runImagingScienceFieldTrimmed.sh


    fi

    cd ..

fi

