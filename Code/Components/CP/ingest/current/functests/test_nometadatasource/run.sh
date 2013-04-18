#!/bin/bash

cd `dirname $0`

# Setup the environment
source ../../init_package_env.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

# Run the ingest pipeline
mpirun -np 2 ../../apps/cpingest.sh -c cpingest.in
