#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/frontend/trunk/init_package_env.sh

# Start the Ice Services
../start_services.sh config.icegrid

# Start the runtime
echo "Starting the Central Processor Frontend Runtime..."
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add cpfe_runtime.xml"
sleep 2

# Run the test
$ASKAP_ROOT/Code/Components/CP/frontend/trunk/apps/tConfig.sh --Ice.Config=config.tConfig -inputs workflow.in
STATUS=$?

# Stop the Ice Services
../stop_services.sh config.icegrid

exit $STATUS
