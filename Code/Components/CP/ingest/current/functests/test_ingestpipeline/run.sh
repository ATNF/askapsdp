#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/ingest/current/init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the ingest pipeline
echo "Running the testcase..."
mpirun -np 2 $ASKAP_ROOT/Code/Components/CP/ingest/current/apps/cpingest.sh -inputs cpingest.in
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
