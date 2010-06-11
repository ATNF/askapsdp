#!/bin/bash

# Setup the environment
source ../init_package_env.sh

# Request IceGrid shutdown and wait
echo -n "Stopping IceGrid and IceStorm..."
icegridadmin --Ice.Config=config.icegrid -u foo -p bar -e "node shutdown Node1"
echo "STOPPED"

# Remove temporary directories
rm -rf data icegrid.stdout icegrid.stderr
