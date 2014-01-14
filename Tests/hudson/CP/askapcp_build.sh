#!/bin/bash -l

#
# Bootstrap
#
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice python bootstrap.py
if [ $? -ne 0 ]; then
    echo "Error: Bootstrapping failed"
    exit 1
fi

#
# Init ASKAP environment
#
source initaskap.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current
source initaskap.sh

#
# Build CP
#
nice rbuild -a -M -S -T Code/Systems/cpapps
if [ $? -ne 0 ]; then
    echo "Error: rbuild -a -M -S -T Code/Systems/cpapps"
    exit 1
fi
