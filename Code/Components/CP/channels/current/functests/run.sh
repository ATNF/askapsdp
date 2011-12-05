#!/bin/bash

cd `dirname $0`

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# test_eventchannel testcase
cd test_eventchannel
./run.sh
if [ $? -eq 0 ]; then
    R1="test_eventchannel  PASS"
else
    R1="test_eventchannel  FAIL"
    FAIL=1
fi
cd $INITIALDIR

# test_uvchannel testcase
cd test_uvchannel
./run.sh
if [ $? -eq 0 ]; then
    R2="test_uvchannel  PASS"
else
    R2="test_uvchannel  FAIL"
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
