#!/bin/bash -l

# Nice the processes running on the head node, since we don't really
# want then to interfere with jobs running if possible
NICE="nice -n 10"

unset ASKAP_ROOT
cd $WORKSPACE/trunk
$NICE /usr/bin/python2.6 bootstrap.py
if [ $? -ne 0 ]; then
    echo "Error: Bootstrapping failed"
    exit 1
fi

#
# Phase 2
#
# Nice the processes running on the head node, since we don't really
# want then to interfere with jobs running if possible
NICE="nice -n 10"

#
# Setup environment
#
cd $WORKSPACE/trunk
source initaskap.sh

cd $ASKAP_ROOT

#
# Build
#
$NICE rbuild -n Code/Components/Synthesis/synthesis/current
if [ $? -ne 0 ]; then
   echo "rbuild Code/Components/Synthesis/synthesis/current return !=0"
   exit 1
fi

#
# Phase 3
#

# Nice the processes running on the head node, since we don't really
# want then to interfere with jobs running if possible
NICE="nice -n 10"

cd $WORKSPACE/trunk
source initaskap.sh

# Obtain the test package
cd $WORKSPACE
$NICE tar zxvf /exported2/hudson-test-packages/cimager_mfssmall.tgz
if [ $? -ne 0 ]; then
  exit 1
fi

# Run the job
cd $WORKSPACE/mfssmall
$NICE ./mfssmall.sh
STATUS=$?
if [ $STATUS -ne 0 ]; then
  echo "Error: mfssmall batch job returned != 0"
  exit $STATUS
fi
