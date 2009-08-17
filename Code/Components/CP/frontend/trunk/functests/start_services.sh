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
ICE_ROOT=$ASKAP_ROOT/3rdParty/Ice/tags/Ice-3.3.0/install
export PATH=$PATH:$ICE_ROOT/bin

# For Unix/Linux
export LD_LIBRARY_PATH=$ICE_ROOT/lib:$LD_LIBRARY_PATH

# For Mac OSX
export DYLD_LIBRARY_PATH=$ICE_ROOT/lib:$DYLD_LIBRARY_PATH

# Create directories for IceGrid and IceStorm
mkdir -p data/registry
mkdir -p data/node

# Start the Ice Grid
$ICE_ROOT/bin/icegridnode --Ice.Config=$1 > /dev/null 2>&1 &
ICEGRIDPID=$!
echo $ICEGRIDPID > icegrid.pid 

waitIceGrid

# Add the IceStorm service
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add icestorm.xml" > /dev/null 2>&1

waitIceStorm


