#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the test harness
echo "Running the testcase..."
../../apps/tMetadataSource.sh -inputs tMetadataSource.in
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
