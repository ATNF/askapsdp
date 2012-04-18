#!/bin/bash

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
function terminate_process {
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

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
source ../common.sh

# Remove the log files
CPMAN_LOG=cpmanager.log
REG_LOG=icegridregistry.log
rm -f ${CPMAN_LOG} ${REG_LOG}

# Start the Ice Registry
echo "Starting the Ice Registry..."
REG_DB=registry-db
rm -rf ${REG_DB}
mkdir -p ${REG_DB}
nohup icegridregistry --Ice.Config=config.icegridregistry > ${REG_LOG} &
REG_PID=$!
waitIceRegistry config.icegridadmin

# Start the cpmanager
echo "Starting the CP Manager..."
nohup java askap/cp/manager/CpManager --Ice.Config=config.cpmanager > ${CPMAN_LOG} &
PID=$!
waitIceAdapter config.icegridadmin CentralProcessorAdminAdapter

# Run the test
echo "Executing the testcase..."
python test_transitions.py --Ice.Config=config.icegridadmin
STATUS=$?

# Stop the service under test
terminate_process ${PID}

# Stop the Ice Registry
terminate_process ${REG_PID}

# Cleanup
rm -rf ${REG_DB}

exit $STATUS
