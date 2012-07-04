#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
source ../common.sh

# Remove the log files
APP_LOG=skymodelsvc.log
REG_LOG=icegridregistry.log
rm -f ${APP_LOG} ${REG_LOG}

# Start the Ice Registry
echo "Starting the Ice Registry..."
REG_DB=registry-db
rm -rf ${REG_DB}
mkdir -p ${REG_DB}
nohup icegridregistry --Ice.Config=icegridregistry.cfg > ${REG_LOG} &
REG_PID=$!
waitIceRegistry icegridadmin.cfg

# Start the service under test
echo "Starting the Sky Model Service..."
nohup java -Xmx1024m askap/cp/skymodelsvc/Server --Ice.Config=skymodelsvc.cfg > ${APP_LOG} &
PID=$!
waitIceAdapter icegridadmin.cfg SkyModelServiceAdapter

# Run the test
echo "Executing the testcase..."
python test_service.py --Ice.Config=icegridadmin.cfg
STATUS=$?

# Stop the service under test
terminate_process ${PID}

# Stop the Ice Registry
terminate_process ${REG_PID}

# Cleanup
rm -rf ${REG_DB}

exit $STATUS
