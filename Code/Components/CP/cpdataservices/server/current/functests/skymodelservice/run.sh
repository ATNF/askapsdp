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
nohup icegridregistry --Ice.Config=config.icegridregistry > ${REG_LOG} &
REG_PID=$!
waitIceRegistry config.icegridadmin

# Start the service under test
echo "Starting the Sky Model Service..."
nohup java -Xmx1024m askap/cp/skymodelsvc/Server --Ice.Config=config.skymodelsvc > ${APP_LOG} &
PID=$!
waitIceAdapter config.icegridadmin SkyModelServiceAdminAdapter

# Run the test
echo "Executing the testcase..."
python test_transitions.py --Ice.Config=config.icegridadmin
STATUS=$?

# Stop the service under test
kill -SIGTERM $PID
sleep 2
kill -SIGKILL $PID > /dev/null 2>&1

# Stop the Ice Registry
kill -SIGTERM $REG_PID
sleep 2
kill -SIGKILL $REG_PID > /dev/null 2>&1

# Cleanup
rm -rf ${REG_DB}

exit $STATUS
