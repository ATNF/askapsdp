#!/bin/bash -l

imScripts=`pwd`/imagingScripts

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

    . ${imScripts}/config.sh
    workdir="imaging_run_${now}"
    mkdir -p ${workdir}
    cd ${workdir}
    mkdir -p ${parsetdir}
    mkdir -p ${logdir}

    # Get calibration parameters using the calibration MSs
    . ${imScripts}/calibrate.sh

    # Combine calibration parameters
    . ${imScripts}/combineCalResults.sh

    # Create a coarse-channel (1MHz-resolution) MS
    . ${imScripts}/create-coarse-ms.sh

    # Image the science field
    . ${imScripts}/imageScienceField.sh

fi

