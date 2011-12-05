#!/bin/bash

cd `dirname $0`

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# Non-cycling Continuum Imager Test
cd TestIceAppender
./run.sh
if [ $? -eq 0 ]; then
    R1="TestIceAppender     PASS"
else
    R1="TestIceAppender     FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Print Results
echo
echo Result Summary:
echo ============================
echo $R1

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
