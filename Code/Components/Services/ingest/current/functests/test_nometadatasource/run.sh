#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

# Run the ingest pipeline
../../apps/cpingest.sh -s -c cpingest.in
