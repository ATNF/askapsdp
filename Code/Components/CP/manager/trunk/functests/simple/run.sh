#!/bin/bash

# Setup the environment
ICE_ROOT=$ASKAP_ROOT/3rdParty/Ice/tags/Ice-3.3.0/install
export PATH=$PATH:$ICE_ROOT/bin

# For Unix/Linux
export LD_LIBRARY_PATH=$ICE_ROOT/lib:$LD_LIBRARY_PATH

# For Mac OSX
export DYLD_LIBRARY_PATH=$ICE_ROOT/lib:$DYLD_LIBRARY_PATH

# Remove the IceGrid log files
rm -f icegrid.stdout
rm -f icegrid.stderr

# Start the Ice Services
../start_services.sh config.icegrid

# Start the cpmanager
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add cpmanager.xml"
sleep 2

# Run the test
#$ASKAP_ROOT/Code/Components/CP/manager/trunk/apps.....
#STATUS=$?
STATUS=0

# Remove the cpmanager
$ICE_ROOT/bin/icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application remove cpmanager"

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
