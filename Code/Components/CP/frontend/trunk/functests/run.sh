#!/bin/bash

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# Simple testcase
cd simple
./run.sh
if [ $? -eq 0 ]; then
    R1="simple      PASS"
else
    R1="simple      FAIL"
    FAIL=1
fi
cd $INITIALDIR

# test_ports testcase
cd test_ports
./run.sh
if [ $? -eq 0 ]; then
    R2="test-ports  PASS"
else
    R2="test-ports  FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Print Results
echo
echo Result Summary:
echo ===============
echo $R1
echo $R2

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
