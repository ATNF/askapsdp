#!/bin/bash

cd `dirname $0`

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# test_metadatasource testcase
cd test_metadatasource
./run.sh
if [ $? -eq 0 ]; then
    R1="test_metadatasource  PASS"
else
    R1="test_metadatasource  FAIL"
    FAIL=1
fi
cd $INITIALDIR

# test_vissource testcase
cd test_vissource
./run.sh
if [ $? -eq 0 ]; then
    R2="test_vissource  PASS"
else
    R2="test_vissource  FAIL"
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
