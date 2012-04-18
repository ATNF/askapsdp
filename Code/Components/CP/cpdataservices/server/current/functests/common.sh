#!/bin/bash

# Arg1: icegrid admin config file name
waitIceRegistry()
{ # Wait for IceGrid to start, but timeout after 10 seconds
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

# Arg1: icegrid admin config file name
# Arg2: Name of adapter to wait for
waitIceAdapter()
{ # Wait for IceGrid to start, but timeout after 10 seconds
    echo -n Waiting for Adapter $2 to be available...
    TIMEOUT=10
    STATUS=1
    while [ $STATUS -ne 0 ] && [ $TIMEOUT -ne 0 ]; do
        icegridadmin --Ice.Config=$1 -u foo -p bar -e "adapter endpoints $2" > /dev/null 2>&1
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

# Arg1: Process ID
function process_exists {
    ps -p $1 > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        PROC_EXISTS=true
    else
        PROC_EXIST=false
    fi
}

# Arg1: Process ID
function terminate_process  {
    PID=$1
    process_exists ${PID}
    if ${PROC_EXISTS}; then
        kill -SIGTERM ${PID}
        # Wait
        sleep 5
        process_exists ${PID}
        if ${PROC_EXISTS} ; then
            kill -SIGKILL ${PID} > /dev/null 2>&1
        fi
    fi
}

