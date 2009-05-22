#!/bin/bash

# Create directories for IceGrid and IceStorm
mkdir -p data/registry
mkdir -p data/node

# Start the Ice Grid
icegridnode --Ice.Config=config.icegrid > /dev/null 2>&1 &
ICEGRIDPID=$!

# TODO: Find a method of waiting for the icegrid to startup so we
# don't need to do this sleep
sleep 1

# Add the IceStorm service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add icestorm.xml" > /dev/null 2>&1

# TODO: Find a method of waiting for icestorm to startup so we
# don't need to do this sleep
sleep 1

# Run the test
$ASKAP_ROOT/Code/Base/logappenders/trunk/apps/tIceAppender.sh --Ice.Config=config.tIceAppender

# Request IceGrid shutdown and wait
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node shutdown Node1"
wait $ICEGRIDPID

# Remove temporary directories
rm -rf data
