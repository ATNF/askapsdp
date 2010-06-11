#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/ingest/trunk/init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Components/Synthesis/testdata/trunk

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the ingest pipeline
echo "Running the testcase..."
$ASKAP_ROOT/Code/Components/CP/ingest/trunk/apps/cpingest.sh -inputs cpingest.in
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
