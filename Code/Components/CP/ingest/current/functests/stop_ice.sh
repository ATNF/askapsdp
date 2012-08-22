#!/usr/bin/env bash

# Arg1: Process ID
function process_exists {
    ps -p $1 > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        PROC_EXISTS=true
    else
        PROC_EXIST=false
    fi
}

# Arg1: Pidfile name
function kill_service  {
    echo -n "Terminating service: ${1}..."
    if [ -f ${1}.pid ]; then
        PID=`cat ${1}.pid`
        process_exists ${PID}
        if ${PROC_EXISTS} ; then
            kill -SIGTERM ${PID}
            sleep 5
            kill -SIGKILL ${PID} > /dev/null 2>&1
        fi
        rm -f ${1}.pid
    fi
    echo "Done"
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
source `dirname $0`/../init_package_env.sh

# Stop IceStorm
kill_service icestorm

echo -n "Terminating service: iceregistry..."
# Request Ice Registry shutdown
icegridadmin --Ice.Config=$1 -u foo -p bar -e "registry shutdown Master"
if [ $? -ne 0 ]; then
    echo "Error stopping registry"
else
    echo "Done"
fi
