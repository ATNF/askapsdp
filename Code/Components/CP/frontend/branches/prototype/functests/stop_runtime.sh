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
source $ASKAP_ROOT/Code/Components/CP/frontend/trunk/init_package_env.sh

echo -n "Stopping the cpfe_runtime..."
# Request IceGrid shutdown and wait
icegridadmin --Ice.Config=$1 -u foo -p bar -e "application remove cpfe_runtime"
sleep 2
echo "STOPPED"
