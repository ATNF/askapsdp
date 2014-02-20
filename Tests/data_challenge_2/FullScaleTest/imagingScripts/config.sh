#!/bin/bash -l

# General

slicer=${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/makeModelSlice.sh
createFITS=${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/createFITS.sh
rndgains=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/randomgains.sh
csim=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/csimulator.sh
ccal=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh
cim=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh
mssplit=${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/mssplit.sh
msmerge=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/msmerge.sh
askapconfig=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/stdtest/definitions

now=`date +%F-%H%M`

parsetdir=parsets
logdir=logs


doCal=true

#################
#  Ccalibrator parameters

# Position of 1934-638 - assume this is the calibrator
ra1934=294.854275
ra1934str=19h39m25.036
dec1934=-63.712675
dec1934str=-63.42.45.63
direction1934="[${ra1934str}, ${dec1934str}, J2000]"

splitMSbeforeCal=false

# Parameters for ccalibrator
ncycCal=10
calInterval=1800
calSolveNbeam=9
calSolve=gains
calDirection=${direction1934}

calGridder=AWProject
wmaxCal=800
nwCal=129
osCal=4

inputCalMSbase=calibrator_J1934m638_2014-02-20-1203

###################
# Imaging parameters

CONT_CLEAN_MPPWIDTH=913
CONT_CLEAN_MPPNPPN=16
CONT_CLEAN_FREQ=0.9e9

# image size -- number of pixels and cellsize
IMAGING_NUM_PIXELS=2048
IMAGING_CELLSIZE=10arcsec
IMAGING_DIRECTION="[12h30m00.00, -45.00.00.00, J2000]"

# gridding parameters for imaging 
IMAGING_WTOL=800
IMAGING_WMAX=800
IMAGING_NWPLANES=99
IMAGING_OVERSAMPLE=4
IMAGING_MAXSUP=512
IMAGING_GAUSSTAPER="[30arcsec, 30arcsec, 0deg]"
IMAGING_EQUALISE=True

imagebase=image.i.clean.sciencefield.SKADS
scienceMS=sciencefield_SKADS_2014-02-20-1203.ms


