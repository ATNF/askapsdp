#!/bin/bash

# Setup the environment
ICE_ROOT=$ASKAP_ROOT/3rdParty/Ice/tags/Ice-3.3.0/install
export PATH=$PATH:$ICE_ROOT/bin

# For Unix/Linux
export LD_LIBRARY_PATH=$ICE_ROOT/lib:$LD_LIBRARY_PATH

# For Mac OSX
export DYLD_LIBRARY_PATH=$ICE_ROOT/lib:$DYLD_LIBRARY_PATH

../start_services.sh config.icegrid

# Run the test
$ASKAP_ROOT/Code/Components/CP/frontend/trunk/apps/tActivities.sh --Ice.Config=config.tActivities
STATUS=$?

../stop_services.sh config.icegrid

exit $STATUS
