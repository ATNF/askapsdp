#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Run the test
$ASKAP_ROOT/Code/Components/CP/correlatorsim/trunk/apps/playback.sh -inputs playback.in
STATUS=$?

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
