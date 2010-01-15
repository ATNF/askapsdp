#!/bin/bash

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# test_ports testcase
cd test_ports
./run.sh
if [ $? -eq 0 ]; then
    R1="test-ports  PASS"
else
    R1="test-ports  FAIL"
    FAIL=1
fi
cd $INITIALDIR

# test_activities testcase
cd test_activities
./run.sh
if [ $? -eq 0 ]; then
    R2="test-activities  PASS"
else
    R2="test-acivities  FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Simple testcase
cd control
./run.sh
if [ $? -eq 0 ]; then
    R3="control      PASS"
else
    R3="control      FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Simple testcase
cd config
./run.sh
if [ $? -eq 0 ]; then
    R4="config      PASS"
else
    R4="config      FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Print Results
echo
echo Result Summary:
echo ===============
echo $R1
echo $R2
echo $R3
echo $R4

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
