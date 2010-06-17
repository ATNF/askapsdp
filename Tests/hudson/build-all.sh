#!/bin/bash -l

# Bootstrap.
unset ASKAP_ROOT
cd $WORKSPACE/trunk
/usr/bin/python2.6 bootstrap.py
if [ $? -ne 0 ]; then
    exit 1
fi

# Setup ASKAP environment.
source initaskap.sh

# Build 3rdParty.
cd $ASKAP_ROOT/3rdParty
python autobuild.py -q -x install
if [ $? -ne 0 ]; then
    exit 1
fi

# Build Code.
cd $ASKAP_ROOT/Code
python autobuild.py -q -x install
if [ $? -ne 0 ]; then
    exit 1
fi

# Build documentation in Code.
cd $ASKAP_ROOT/Code
python autobuild.py -q doc
if [ $? -ne 0 ]; then
    exit 1
fi
