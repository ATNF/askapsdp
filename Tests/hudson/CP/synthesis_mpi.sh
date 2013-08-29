#!/bin/bash

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
# Setup environment
#
source /etc/profile.d/modules.sh
source initaskap.sh
if [ $MODULESHOME ]; then
    module load openmpi
else
    echo 'Error: $MODULESHOME is not defined'
    exit 1
fi


#
# Build Synthesis
#
$NICE  rbuild -n -p mpich=1 Code/Components/Synthesis/synthesis/current
if [ $? -ne 0 ]; then
    echo "Error: rbuild -n Code/Components/Synthesis/synthesis/current
failed "
    exit 1
fi

#
# Build testdata for the unit tests
#
$NICE rbuild -n -p mpich=1 Code/Base/accessors/current
if [ $? -ne 0 ]; then
    echo "rbuild -n Code/Base/accessors/current failed"
    exit 1
fi

#
# Run the unit tests
#
cd $WORKSPACE/trunk/Code/Components/Synthesis/synthesis/current
$NICE scons test
exit 0
