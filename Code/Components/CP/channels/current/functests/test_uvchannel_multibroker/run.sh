#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh

# Run the uvchannel test 
echo "Running the testcase..."
../../apps/tUVChannel.sh -c tUVChannel.in
STATUS=$?
echo "Testcase finished"

exit $STATUS
