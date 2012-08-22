#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh

# Start the Ice Services
../start_ice.sh ../iceregistry.cfg ../icegridadmin.cfg ../icestorm.cfg
sleep 1

# Run the test harness
echo "Running the testcase..."
../../apps/tMetadataSource.sh -inputs tMetadataSource.in
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_ice.sh ../icegridadmin.cfg

exit $STATUS
