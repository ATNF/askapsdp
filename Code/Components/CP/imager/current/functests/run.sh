#!/bin/bash

cd `dirname $0`

INITIALDIR=`pwd`
echo Running test cases...

FAIL=0

# Non-cycling Continuum Imager Test
cd continuum
./run.sh
if [ $? -eq 0 ]; then
    R1="continuum           PASS"
else
    R1="continuum           FAIL"
    FAIL=1
fi
cd $INITIALDIR

# Cycling Continuum Imager Test
cd continuum-cycling
./run.sh
if [ $? -eq 0 ]; then
    R2="continuum-cycling   PASS"
else
    R2="continuum-cycling   FAIL"
    FAIL=1
fi
cd $INITIALDIR

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
