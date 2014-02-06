#!/bin/bash -l

#
# Bootstrap
#
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice python bootstrap.py -n
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

cd Code/Systems/cpapps

#
# Build CP applications
#
nice rbuild -a -M -S -T
if [ $? -ne 0 ]; then
    echo "Error: rbuild -a -M -S -T"
    exit 1
fi

#
# Build a release tarball
#
nice rbuild -n  -M -S -T -t release 
if [ $? -ne 0 ]; then
    echo "Error: rbuild rbuild -n  -M -S -T -t release"
    exit 1
fi

# Need a "standard name" for the artifact
mv release-*.tgz cpapps-release.tgz
