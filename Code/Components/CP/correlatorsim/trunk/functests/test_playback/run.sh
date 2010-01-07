#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Start the metadata subscriber
$ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/apps/tMetadata.sh --Ice.Config=config.tMetadata > tMetadata.log 2>&1 &
MDPID=$!

# Run the test
$ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/apps/playback.sh -inputs playback.in
STATUS=$?
sleep 1

# Give the subscriber a moment to get the last messages then exit
sleep 1
kill $MDPID
sleep 1
kill -9 $MDPID > /dev/null 2>&1

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
