#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/cpdataservices/server/current/init_package_env.sh

# Remove the IceGrid log files
rm -f icegrid.stdout
rm -f icegrid.stderr

# Start the Ice Services
../start_services.sh config.icegrid

# Start the sky model service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add skymodelservice.xml"
sleep 2

# Run the test
python test_transitions.py --Ice.Config=config.icegridadmin
STATUS=$?

# Remove the sky model service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application remove skymodelservice"

# Stop the Ice Services
../stop_services.sh config.icegrid

# Cleanup
rm -f CommonTypes_ice.py Component_ice.py CommonTypes_ice.pyc Component_ice.pyc
rm -rf askap

exit $STATUS
