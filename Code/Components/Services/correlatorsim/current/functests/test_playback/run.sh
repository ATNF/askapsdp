#!/bin/bash

cd `dirname $0`

# Setup the environment
if [ ! -f ../../init_package_env.sh ]; then
    echo "Error: init_package_env.sh dos not exist, please run rbuild in package dir"
    exit 1
fi
source ../../init_package_env.sh

# Start the Ice Services
../start_ice.sh ../iceregistry.cfg ../icegridadmin.cfg ../icestorm.cfg
if [ $? -ne 0 ]; then
    echo "Error: Failed to start Ice Services"
    exit 1
fi
sleep 1

# Start the metadata subscriber (don't use the script so this script can kill it)
../../apps/msnoop -c msnoop.in -v > msnoop.log 2>&1 &
MDPID=$!

# Start the visibilities receiver (don't use the script so this script can kill it)
../../apps/vsnoop -v -p 3000 > vsnoop1.log 2>&1 &
VISPID1=$!

# Run the test
mpirun -np 3 ../../apps/playback.sh -c playback.in
STATUS=$?

# Give the subscriber a moment to get the last messages then exit
sleep 5
kill $MDPID $VISPID1 $VISPID2
sleep 1
kill -9 $MDPID $VISPID $VISPID2 > /dev/null 2>&1

# Stop the Ice Services
../stop_ice.sh ../icegridadmin.cfg

exit $STATUS
