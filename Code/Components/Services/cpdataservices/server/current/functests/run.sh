#!/bin/bash

cd `dirname $0`
INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

cd skymodelservice
./run.sh
if [ $? -eq 0 ]; then
    R1="skymodelservice     PASS"
else
    R1="skymodelservice     FAIL"
    FAIL=1
fi
cd $INITIALDIR

cd caldataservice
./run.sh
if [ $? -eq 0 ]; then
    R2="caldataservice     PASS"
else
    R2="caldataservice     FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Print Results
echo
echo Result Summary:
echo ============================
echo $R1
echo $R2

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
