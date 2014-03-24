#!/bin/bash

cd `dirname $0`

# Set a 16GB vmem limit
ulimit -v 16777216

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/ingest/current/init_package_env.sh
#export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current
export AIPSPATH=/data/AKSCOR01_1/work/vor010/measures_data

# Run the ingest pipeline
exec $ASKAP_ROOT/Code/Components/CP/ingest/current/apps/cpingest.sh -s -c ./cpingest.in
