#!/bin/bash

waitIceGrid()
{ # Wait for IceGrid to start, but timeout after 10 seconds
    echo -n Waiting for IceGrid to startup...
    TIMEOUT=10
    STATUS=1
    while [ $STATUS -ne 0 ] && [ $TIMEOUT -ne 0 ]; do
        $ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node list" > /dev/null 2>&1
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
        RESULT=`$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "adapter endpoints IceStorm.TopicManager" > /dev/null 2>&1`
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
source ../../init_package_env.sh

# Create directories for IceGrid and IceStorm
mkdir -p data/registry
mkdir -p data/node

# Start the Ice Grid
$ICE_ROOT/bin/icegridnode --Ice.Config=config.icegrid > /dev/null 2>&1 &
ICEGRIDPID=$!

waitIceGrid

# Add the IceStorm service
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add icestorm.xml" > /dev/null 2>&1

waitIceStorm

# Run the test
java -cp ../../install/lib/java-logappenders.jar:$ASKAP_ROOT/lib/log4j-1.2.15.jar:$ASKAP_ROOT/lib/Ice.jar askap.TestIceAppender TestIceAppender.log_cfg --Ice.Config=config.tIceAppender
STATUS=$?

# Request IceGrid shutdown and wait
echo -n "Stopping IceGrid and IceStorm..."
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node shutdown Node1"
wait $ICEGRIDPID
echo "STOPPED"

# Remove temporary directories
rm -rf data

exit $STATUS
