#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the component under test (metadata bridge)
#$ASKAP_ROOT/Code/Components/CP/bridges/trunk/apps/mdbridge.sh -inputs mdbridge.in

# Run the test harness
$ASKAP_ROOT/Code/Components/CP/bridges/trunk/apps/tmdbridge.sh
STATUS=$?

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
