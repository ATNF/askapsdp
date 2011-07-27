#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/manager/current/init_package_env.sh

# Remove the IceGrid log files
rm -f icegrid.stdout
rm -f icegrid.stderr

# Start the Ice Services
../start_services.sh config.icegrid

# Start the cpmanager
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add cpmanager.xml"
sleep 2

# Run the test
python test_transitions.py --Ice.Config=config.icegridadmin
STATUS=$?

# Remove the cpmanager
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application remove cpmanager"

# Stop the Ice Services
../stop_services.sh config.icegrid

# Cleanup
rm -f CommonTypes_ice.py Component_ice.py CommonTypes_ice.pyc Component_ice.pyc
rm -rf askap

exit $STATUS
