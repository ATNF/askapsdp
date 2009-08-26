#!/bin/bash

# Check the config file has been passed
if [ $# -ne 1 ]; then
    echo "usage: $0 <config file>"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "Error: Config file $1 not found"
    exit 1
fi

# Setup the environment
ICE_ROOT=$ASKAP_ROOT/3rdParty/Ice/tags/Ice-3.3.0/install
export PATH=$PATH:$ICE_ROOT/bin

# For Unix/Linux
export LD_LIBRARY_PATH=$ICE_ROOT/lib:$LD_LIBRARY_PATH

# For Mac OSX
export DYLD_LIBRARY_PATH=$ICE_ROOT/lib:$DYLD_LIBRARY_PATH

echo -n "Stopping IceGrid..."
# Request IceGrid shutdown and wait
$ICE_ROOT/bin/icegridadmin --Ice.Config=$1 -u foo -p bar -e "node shutdown Node1"
sleep 2
echo "STOPPED"

# Remove temporary directories
rm -rf data
