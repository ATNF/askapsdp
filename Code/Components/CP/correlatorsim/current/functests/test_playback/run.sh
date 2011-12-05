#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid
sleep 2

# Start the metadata subscriber (don't use the script so this script can kill it)
../../apps/msnoop -inputs msnoop.in > msnoop.log 2>&1 &
MDPID=$!

# Start the visibilities receiver (don't use the script so this script can kill it)
../../apps/vsnoop -v -p 3000 > vsnoop1.log 2>&1 &
VISPID1=$!

# And for the second correlator shelf
../../apps/vsnoop -v -p 3001 > vsnoop2.log 2>&1 &
VISPID2=$!

# Run the test
mpirun -np 3 ../../apps/playback.sh -inputs playback.in
STATUS=$?

# Give the subscriber a moment to get the last messages then exit
sleep 5
kill $MDPID $VISPID1 $VISPID2
sleep 1
kill -9 $MDPID $VISPID $VISPID2 > /dev/null 2>&1

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
