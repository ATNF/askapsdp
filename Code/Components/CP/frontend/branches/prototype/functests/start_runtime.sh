#!/bin/bash

waitRuntime()
{ # Wait for cpfe_runtime to start, but timeout after 10 seconds
    echo -n Waiting for cpfe_runtime to startup...
    TIMEOUT=10
    STATUS=1
    INACTIVE="<inactive>"
    RESULT=$INACTIVE
    while [ $STATUS -ne 0 ] && [ "${RESULT}" = "${INACTIVE}" ] && [ $TIMEOUT -ne 0 ]; do
        RESULT=`icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "adapter endpoints CpfeRuntimeAdapter" > /dev/null 2>&1`
        STATUS=$?
        TIMEOUT=`expr $TIMEOUT - 1`
        sleep 1
    done

    if [ $TIMEOUT -eq 0 ]; then
        echo FAILED
    else
        echo STARTED
    fi
} 

# Check the config file has been passed
if [ $# -ne 1 ]; then
    echo "usage: $0 <config file>"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "Error: Config file $1 not found"
    exit 1
fi

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/frontend/trunk/init_package_env.sh

# Add the IceStorm service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add cpfe_runtime.xml" > /dev/null 2>&1

waitRuntime
