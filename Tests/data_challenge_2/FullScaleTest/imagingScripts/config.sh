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

#calGridder=AWProject
calGridder=SphFunc
wmaxCal=800
nwCal=129
osCal=4

#inputCalMSbase=../MS/calibrator_J1934m638_2014-02-20-1203
#inputCalMSbase=../MS/calibrator_J1934m638_30s_2014-02-24-0836
#inputCalMSbase=../MS/calibrator_J1934m638_5s_2014-02-24-1911
#inputCalMSbase=../MS/calibrator_J1934m638_5s_2014-03-05-1050
inputCalMSbase=../MS/calibrator_J1934m638_5s_2014-03-06-0857

###################
# Imaging parameters

CONT_CLEAN_FREQ=0.9e9

# image size -- number of pixels and cellsize
IMAGING_CELLSIZE=10arcsec

model=SKADS
#model=BETAtestfield


if [ $model == "SKADS" ]; then

    doMFS=true
    CONT_CLEAN_MPPWIDTH=913
    CONT_CLEAN_MPPNPPN=20

    IMAGING_DIRECTION="[12h30m00.00, -45.00.00.00, J2000]"
    IMAGING_NUM_PIXELS=2048
    IMAGING_MAXSUP=512
    IMAGING_WTOL=2600
    IMAGING_WMAX=2600
#scienceMS=../MS/sciencefield_SKADS_2014-02-20-1203.ms
#scienceMS=../MS/sciencefield_SKADS_30s_2014-02-24-0836.ms
    scienceMS=../MS/sciencefield_SKADS_5s_2014-02-24-1911.ms
    imagebase=image.i.clean.sciencefield.SKADS

else
    
    doMFS=false
    CONT_CLEAN_MPPWIDTH=305
    CONT_CLEAN_MPPNPPN=20

    IMAGING_DIRECTION="[15h56m58.870,-79.14.04.28, J2000]"
    IMAGING_NUM_PIXELS=3072
    IMAGING_MAXSUP=1024
    IMAGING_WTOL=1600
    IMAGING_WMAX=1600
    scienceMS=../MS/sciencefield_BETAtestfield_5s_2014-03-05-1050.ms
    imagebase=image.i.clean.sciencefield.BETAtestfield

fi

# gridding parameters for imaging 
#IMAGING_GRIDDER=AWProject
IMAGING_GRIDDER=WProject
IMAGING_NWPLANES=99
IMAGING_OVERSAMPLE=4
#
IMAGING_GAUSSTAPER="[30arcsec, 30arcsec, 0deg]"
IMAGING_EQUALISE=True
#IMAGING_CLEANSCALES="[0, 3, 10, 30]"
IMAGING_CLEANSCALES="[0, 3, 10]"

doAverageMS=false
if [ $doAverageMS == true ]; then
    coarseMS=MS/coarseChan.ms
else
    coarseMS=`echo $scienceMS | sed -e 's/\.ms/-COARSE.ms/g'`
fi
END_CHANNEL_CREATECOARSE=16416


