#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/bridges/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the test harness
echo "Running the testcase..."
$ASKAP_ROOT/Code/Components/CP/ingest/trunk/apps/tMetadataSource.sh
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
