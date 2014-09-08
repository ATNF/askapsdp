#!/bin/bash -l

simScripts=`pwd`/simulationScripts

doSubmit=true
#doSubmit=false
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
    . ${simScripts}/setup.sh

    # Run the 9-beam calibration observation of 1934-638
    . ${simScripts}/observeCalibrator.sh

    # Make the input sky model
    . ${simScripts}/makeInputModel.sh

    # Run the observation of the science field
    . ${simScripts}/observeScienceField.sh

    cd ..

    if [ ${doSubmit} == true ] && [ "${SBATCH_JOBLIST}" ]; then
	scontrol release $SBATCH_JOBLIST
    fi
fi

