#!/bin/bash

# Setup the environment
ICE_ROOT=$ASKAP_ROOT/3rdParty/Ice/tags/Ice-3.3.0/install
export PATH=$PATH:$ICE_ROOT/bin

# For Unix/Linux
export LD_LIBRARY_PATH=$ICE_ROOT/lib:$LD_LIBRARY_PATH

# For Mac OSX
export DYLD_LIBRARY_PATH=$ICE_ROOT/lib:$DYLD_LIBRARY_PATH

# Start the Ice Services
../start_services.sh config.icegrid

# Start the runtime
echo "Starting the Central Processor Frontend Runtime..."
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add cpfe_runtime.xml"
sleep 2

# Run the test
$ASKAP_ROOT/Code/Components/CP/frontend/trunk/apps/tControl.sh --Ice.Config=config.tControl -inputs workflow.in
STATUS=$?

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
