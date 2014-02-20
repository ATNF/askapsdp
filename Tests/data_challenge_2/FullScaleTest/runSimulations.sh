#!/bin/bash -l

simScripts=`pwd`/simulationScripts

#doSubmit=true
doSubmit=false
runIt=true

depend=""

if [ "${ASKAP_ROOT}" == "" ]; then
    echo "You have not set ASKAP_ROOT! Exiting."
    runIt=false
fi

if [ "${AIPSPATH}" == "" ]; then
    echo "You have not set AIPSPATH! Exiting."
    runIt=false
fi

if [ $runIt == true ]; then    

    . ${simScripts}/config.sh
    workdir="run_${now}"
    mkdir -p ${workdir}
    cd ${workdir}
    mkdir -p ${parsetdir}
    mkdir -p ${logdir}
    mkdir -p ${msdir}

    if [ $doCorrupt == true ]; then
	$rndgains -f 9 -a 6 -p 2 $randomgainsparset
    fi


    # Run the 9-beam calibration observation of 1934-638
    . ${simScripts}/observeCalibrator.sh

    # Run the observation of the science field
    . ${simScripts}/observeScienceField.sh

fi

