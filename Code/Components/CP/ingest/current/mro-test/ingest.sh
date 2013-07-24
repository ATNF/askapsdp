#!/bin/bash

cd `dirname $0`

# Set a 2GB vmem limit
ulimit -v 2097152

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/ingest/current/init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

# Run the ingest pipeline
$ASKAP_ROOT/Code/Components/CP/ingest/current/apps/cpingest.sh -s -c ./cpingest.in
