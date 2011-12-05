#!/bin/bash

waitIceGrid()
{ # Wait for IceGrid to start, but timeout after 10 seconds
    echo -n Waiting for IceGrid to startup...
    TIMEOUT=10
    STATUS=1
    while [ $STATUS -ne 0 ] && [ $TIMEOUT -ne 0 ]; do
        icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node list" > /dev/null 2>&1
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
        RESULT=`icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "adapter endpoints IceStorm.TopicManager" > /dev/null 2>&1`
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

# Setup the environment
source `dirname $0`/../../init_package_env.sh

# Create directories for IceGrid and IceStorm
mkdir -p data/registry
mkdir -p data/node

# Start the Ice Grid
icegridnode --Ice.Config=config.icegrid > /dev/null 2>&1 &
ICEGRIDPID=$!

waitIceGrid

# Add the IceStorm service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add icestorm.xml" > /dev/null 2>&1

waitIceStorm

# Run the test
java askap.test.TestIceAppender TestIceAppender.log_cfg --Ice.Config=config.TestIceAppender
STATUS=$?

# Request IceGrid shutdown and wait
echo -n "Stopping IceGrid and IceStorm..."
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node shutdown Node1"
wait $ICEGRIDPID
echo "STOPPED"

# Remove temporary directories
rm -rf data

exit $STATUS
