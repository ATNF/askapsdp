#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
source ../common.sh

# Remove the log files
APP_LOG=caldatasvc.log
REG_LOG=iceregistry.log
rm -f ${APP_LOG} ${REG_LOG}

# Start the Ice Registry
echo "Starting the Ice Registry..."
REG_DB=registry-db
rm -rf ${REG_DB}
mkdir -p ${REG_DB}
nohup icegridregistry --Ice.Config=iceregistry.cfg > ${REG_LOG} 2>&1 &
REG_PID=$!
waitIceRegistry icegridadmin.cfg

# Start the service under test
echo "Starting the Calibration Data Service..."
nohup java -Xmx1024m askap/cp/caldatasvc/Server --Ice.Config=caldatasvc.cfg > ${APP_LOG} 2>&1 &
PID=$!
waitIceAdapter icegridadmin.cfg CalibrationDataServiceAdapter

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
