#!/bin/bash

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# test_channel testcase
cd test_channel
./run.sh
if [ $? -eq 0 ]; then
    R1="test_channel  PASS"
else
    R1="test_channel  FAIL"
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
