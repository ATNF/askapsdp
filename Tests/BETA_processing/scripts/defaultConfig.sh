#!/usr/bin/env bash

if [ "$ASKAP_ROOT" != "" ]; then
    AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current
    mssplit=$ASKAP_ROOT/Code/Components/CP/pipelinetasks/current/apps/mssplit.sh
    cflag=$ASKAP_ROOT/Code/Components/CP/pipelinetasks/current/apps/cflag.sh
    cbpcalibrator=$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/cbpcalibrator.sh
    ccalapply=$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/ccalapply.sh
    cimager=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh
    ccalibrator=$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh
    simager=${ASKAP_ROOT}/Code/Components/CP/simager/current/apps/simager.sh
    linmos=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/linmos.sh
    selavy=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/selavy.sh
else
    mssplit=mssplit
    cflag=cflag
    cbpcalibrator=cbpcalibrator
    ccalapply=ccalapply
    ccalibrator=ccalibrator
    cimager=cimager
    simager=simager
    linmos=linmos
    selavy=selavy
fi

doSubmit=false

CWD=`pwd`
parsets=parsets
logs=logs
slurms=slurmFiles
mkdir -p $parsets
mkdir -p $logs
mkdir -p $slurms

####################
# Control flags
do1934cal=true
doFlag1934=true
doFindBandpass=true

doSci=true
doFlagScience=true
doApplyBandpass=true
doAverageScience=true
doContinuumImaging=true
doSelfcal=false
doSpectralImaging=false
doLinmos=true

####################
# Input Scheduling Blocks (SBs)
# Location of the SBs
SBdir=/scratch/askap/askapops/archive/askap-scheduling-blocks
# SB with 1934-638 observation
SB1934="SET_THIS"
input1934=""
# SB with science observation
SBscience="SET_THIS"
inputSci=""

####################
# Which beams to use.
BEAM_MIN=0
BEAM_MAX=8
nbeam=9


####################
##  BANDPASS CAL

ms1934base=1934_beamBEAM.ms
nchanCBP=16416
chanRange1934="1-16416"
direction1934="[19h39m25.036, -63.42.45.63, J2000]"
bandpassParams=calparameters_1934_bp.tab
ncyclesCBPcal=25

# Dynamic threshold applied to amplitudes [sigma]
cflagDynamicThreshold1934=4.0
# Second amplitude threshold applied [hardware units - before calibration]
cflagAmpThreshold1934=0.2

####################
## Science observation

# This allows selection of particular scans from the science
# observation. If this isn't needed, leave as a blank string.
scanSelectionScience=""
msSciBase=scienceObservation.ms
directionSci=""

chanRangeSci="1-16416"
nchanSci=16416

# Dynamic threshold applied to amplitudes [sigma]
cflagDynamicThresholdSci=4.0
# Second amplitude threshold applied [hardware units - before calibration]
cflagAmpThresholdSci=0.2

chanAverageSci=54

ntermsSci=3
nchanContSci=1
CPUS_PER_CORE_CONTIMG_SCI=20

# base name for images: if sciContImageBase=i.blah then we'll get
# image.i.blah, image.i.blah.restored, psf.i.blah etc
sciContImageBase=i.cont
# number of pixels on the side of the images to be created
pixsizeCont=2048
# Size of the pixels in arcsec
cellsizeCont=10
# Frequency at which continuum image is made [Hz]
freqContSci=863.e6
# Restoring beam: 'fit' will fit the PSF to determine the appropriate
# beam, else give a size
restoringBeamCont=fit

# Interval [sec] over which to solve for self-calibration
intervalSelfcal=10
# Number of loops of self-calibration
numLoopsSelfcal=5
# Should we keep the images from the intermediate selfcal loops?
selfcalKeepImages=true
# SNR threshold for detection with selavy in determining selfcal sources
selavySNRselfcal=15
# Division of image for source-finding in selfcal
selfcalNsubx=6
selfcalNsuby=3

# base name for images: if sciSpectralImageBase=i.blah then we'll get
# image.i.blah, image.i.blah.restored, psf.i.blah etc
sciSpectralImageBase=i.spectral
# number of pixels on the side of the images to be created
pixsizeSpectral=2048
# Size of the pixels in arcsec
cellsizeSpectral=10
# Frequency range for the spectral imager
freqRangeSpectral="713.e6,1013.e6"

# Beam arrangement, used in linmos. If one of "diamond",
# "octagon",... then the positions are filled automatically. Or you
# can use something like (ignoring the ## of course):
##  linmosBeams="linmos.feeds.BEAM0      = [-1.0, -1.0]
##  linmos.feeds.BEAM1      = [-1.0,  0.0]
##  linmos.feeds.BEAM2      = [-1.0,  1.0]
##  linmos.feeds.BEAM3      = [ 0.0, -1.0]
##  linmos.feeds.BEAM4      = [ 0.0,  0.0]
##  linmos.feeds.BEAM5      = [ 0.0,  1.0]
##  linmos.feeds.BEAM6      = [ 1.0, -1.0]
##  linmos.feeds.BEAM7      = [ 1.0,  0.0]
##  linmos.feeds.BEAM8      = [ 1.0,  1.0]"

linmosBeams="diamond"
# Which frequency band are we in - determines beam arrangement (1,2,3,4)
linmosBand=1
# Scale factor for beam arrangement, in format like '1deg'
linmosBeamSpacing="1deg"
# Reference beam for PSF
linmosPSFref=0
