#!/bin/bash -l

# Bootstrap
unset ASKAP_ROOT
cd $WORKSPACE/TOS-0.20
/usr/bin/python2.6 bootstrap.py
if [ $? -ne 0 ]; then
    exit 1
fi

# Setup ASKAP environment
source initaskap.sh

# Build the testcase and dependencies (note the -n is important!!)
rbuild -n Code/Systems/TOS
if [ $? -ne 0 ]; then
    exit 1
fi

# Build TOS-CSS and dependencies
rbuild -n Code/Systems/TOS-CSS
if [ $? -ne 0 ]; then
    exit 1
fi
