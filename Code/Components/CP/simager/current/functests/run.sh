#!/bin/bash

cd `dirname $0`

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# Non-cycling Spectral Line Imager Test
cd spectralline
./run.sh
if [ $? -eq 0 ]; then
    R3="spectralline        PASS"
else
    R3="spectralline        FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Print Results
echo
echo Result Summary:
echo ============================
echo $R1
echo $R2
echo $R3

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
