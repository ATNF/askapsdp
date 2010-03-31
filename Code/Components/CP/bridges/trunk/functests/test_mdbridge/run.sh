#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/bridges/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the component under test (metadata bridge)
#$ASKAP_ROOT/Code/Components/CP/bridges/trunk/apps/mdbridge.sh -inputs mdbridge.in > mdbridge.log 2>&1 &
#MDBRIDGE_PID=$!

# Run the test harness
$ASKAP_ROOT/Code/Components/CP/bridges/trunk/apps/tmdbridge.sh
STATUS=$?

# Stop the component under test
#kill $MDBRIDGE_PID > /dev/null 2>&1
#sleep 2
#kill -9 $MDBRIDGE_PID > /dev/null 2>&1

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
