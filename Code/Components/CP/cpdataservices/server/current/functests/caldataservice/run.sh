#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh

# Remove the IceGrid log files
rm -f icegrid.stdout
rm -f icegrid.stderr

# Start the Ice Services
../start_services.sh config.icegrid

# Start the calibration data service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application add caldataservice.xml"
sleep 2

# Run the test
python test_transitions.py --Ice.Config=config.icegridadmin
STATUS=$?

# Remove the calibration data service
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "application remove caldataservice"

# Stop the Ice Services
../stop_services.sh config.icegrid

# Cleanup
rm -rf askap

exit $STATUS
