#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Start the metadata subscriber (don't use the script so this scripts can kill it)
$ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/apps/tMetadata --Ice.Config=config.tMetadata > tMetadata.log 2>&1 &
MDPID=$!

# Start the visibilities receiver (don't use the script so this scripts can kill it)
$ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/apps/tVisibilities 3000 > tVisibilities.log 2>&1 &
VISPID=$!

# Run the test
$ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/apps/playback.sh -inputs playback.in
STATUS=$?
sleep 1

# Give the subscriber a moment to get the last messages then exit
sleep 5
kill $MDPID $VISPID
sleep 1
kill -9 $MDPID $VISPID > /dev/null 2>&1

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
