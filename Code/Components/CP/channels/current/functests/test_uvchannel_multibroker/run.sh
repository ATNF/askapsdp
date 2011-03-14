#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/channels/current/init_package_env.sh

# Run the uvchannel test 
echo "Running the testcase..."
$ASKAP_ROOT/Code/Components/CP/channels/current/apps/tUVChannel.sh -inputs tUVChannel.in
STATUS=$?
echo "Testcase finished"

exit $STATUS
