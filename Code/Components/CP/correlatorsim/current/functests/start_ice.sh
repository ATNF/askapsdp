#!/usr/bin/env bash

waitIceRegistry()
{ # Wait for Ice Registry to start, but timeout after 10 seconds
    echo -n Waiting for Ice Registry to startup...
    TIMEOUT=10
    STATUS=1
    while [ $STATUS -ne 0 ] && [ $TIMEOUT -ne 0 ]; do
        icegridadmin --Ice.Config=$1 -u foo -p bar -e "registry list" > /dev/null 2>&1
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

waitIceStorm()
{ # Wait for IceGrid to start, but timeout after 10 seconds
    echo -n Waiting for IceStorm to startup...
    TIMEOUT=10
    STATUS=1
    INACTIVE="<inactive>"
    RESULT=$INACTIVE
    while [ $STATUS -ne 0 ] && [ "${RESULT}" = "${INACTIVE}" ] && [ $TIMEOUT -ne 0 ]; do
        RESULT=`icegridadmin --Ice.Config=$1 -u foo -p bar -e "adapter endpoints IceStorm.TopicManager" > /dev/null 2>&1`
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
if [ $# -ne 3 ]; then
    echo "usage: $0 <registry config file> <admin config file> <icestorm config file>"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "Error: IceRegistry config file $1 not found"
    exit 1
fi

if [ ! -f $2 ]; then
    echo "Error: IceGridAdmin config file $2 not found"
    exit 1
fi

if [ ! -f $2 ]; then
    echo "Error: IceStorm config file $2 not found"
    exit 1
fi

# Setup the environment
source `dirname $0`/../init_package_env.sh

# Cleanup/create directories for IceGrid and IceStorm
rm -rf ice_data
mkdir -p ice_data/registry
mkdir -p ice_data/db

# Start the Ice Registry
icegridregistry --Ice.Config=$1 > /dev/null 2>&1 &
ERR=$?
if [ ${ERR} -ne 0 ]; then
    echo "Error: Failed to start ice registry (code ${ERR})"
    exit $ERR
fi

waitIceRegistry $2

# Start IceStorm
nohup icebox --Ice.Config=$3 > /dev/null 2>&1 &
PID=$!
echo "${PID}" > icestorm.pid

waitIceStorm $2
