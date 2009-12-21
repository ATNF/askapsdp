#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/frontend/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid

# Start the runtime
../start_runtime.sh config.icegrid
sleep 2

# Run the test
$ASKAP_ROOT/Code/Components/CP/frontend/trunk/apps/tControl.sh --Ice.Config=config.tControl -inputs workflow.in
STATUS=$?

# Stop the runtime
../stop_runtime.sh config.icegrid

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
