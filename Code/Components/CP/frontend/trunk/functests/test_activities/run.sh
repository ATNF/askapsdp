#!/bin/bash

# Setup the environment
ICE_ROOT=$ASKAP_ROOT/3rdParty/Ice/tags/Ice-3.3.0/install
export PATH=$PATH:$ICE_ROOT/bin

# For Unix/Linux
export LD_LIBRARY_PATH=$ICE_ROOT/lib:$LD_LIBRARY_PATH

# For Mac OSX
export DYLD_LIBRARY_PATH=$ICE_ROOT/lib:$DYLD_LIBRARY_PATH

# Create directories for IceGrid and IceStorm
mkdir -p data/registry
mkdir -p data/node

# Start the Ice Grid
$ICE_ROOT/bin/icegridnode --Ice.Config=config.icegrid > /dev/null 2>&1 &
ICEGRIDPID=$!

# TODO: Find a method of waiting for the icegrid to startup so we
# don't need to do this sleep
sleep 1

# Add the IceStorm service
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add icestorm.xml" > /dev/null 2>&1

# TODO: Find a method of waiting for icestorm to startup so we
# don't need to do this sleep
sleep 1

# Run the test
$ASKAP_ROOT/Code/Components/CP/frontend/trunk/apps/tActivities.sh --Ice.Config=config.tActivities
STATUS=$?

# Request IceGrid shutdown and wait
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node shutdown Node1"
wait $ICEGRIDPID

# Remove temporary directories
rm -rf data

exit $STATUS
