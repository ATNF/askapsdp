#!/bin/bash

INITIALDIR=`pwd`
echo Running test cases...

# Non-cycling Continuum Imager Test
cd continuum
./run.sh
if [ $? -eq 0 ]; then
    R1="continuum\t\tPASS"
else
    R1="continuum\t\tFAIL"
fi
cd $INITIALDIR

# Cycling Continuum Imager Test
cd continuum-cycling
./run.sh
if [ $? -eq 0 ]; then
    R2="continuum-cycling\tPASS"
else
    R2="continuum-cycling\tFAIL"
fi
cd $INITIALDIR

# Non-cycling Spectral Line Imager Test
cd spectralline
./run.sh
if [ $? -eq 0 ]; then
    R3="spectralline\t\tPASS"
else
    R3="spectralline\t\tFAIL"
fi
cd $INITIALDIR

# Print Results
echo
echo Result Summary:
echo ============================
echo $R1
echo $R2
echo $R3
