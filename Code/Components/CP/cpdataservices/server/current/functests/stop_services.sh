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
source `dirname $0`/../init_package_env.sh

echo -n "Stopping IceGrid..."
# Request IceGrid shutdown and wait
icegridadmin --Ice.Config=$1 -u foo -p bar -e "node shutdown Node1"
sleep 2
echo "STOPPED"

# Remove temporary directories
rm -rf data
