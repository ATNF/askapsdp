#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

# Start the Ice Services
../start_services.sh icegrid.cfg
sleep 2

# Run the ingest pipeline
echo "Running the testcase..."
mpirun -np 2 ../../apps/cpingest.sh -inputs cpingest.in
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_services.sh icegrid.cfg

exit $STATUS
