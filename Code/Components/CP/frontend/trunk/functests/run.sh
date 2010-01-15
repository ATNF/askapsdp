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

# Print Results
echo
echo Result Summary:
echo ===============
echo $R1

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
