#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

# Start the Ice Services
../start_ice.sh ../iceregistry.cfg ../icegridadmin.cfg ../icestorm.cfg
if [ $? -ne 0 ]; then
    echo "Error: Failed to start Ice Services"
    exit 1
fi
sleep 1

# Run the ingest pipeline
echo "Running the testcase..."
../../apps/cpingest.sh -s -c cpingest.in
STATUS=$?
echo "Testcase finished"

# Stop the Ice Services
../stop_ice.sh ../icegridadmin.cfg

exit $STATUS
