#!/usr/bin/env bash
#
# The complete set of user-defined parameters, along with their
# default values. All parameters defined herein can be set in the
# input file, and any value given there will override the default
# value set here.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

####################
# Whether to submit the scripts to the queue.
SUBMIT_JOBS=false

####################
# Locations of the executables. 
# If you have a local version of the ASKAPsoft codebase, and have
# defined ASKAP_ROOT, then you will use the executables from that
# tree. 
# Otherwise (the most common case for ACES members), the executables
# from the askapsoft module will be used.

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

####################
# Define & create directories
CWD=`pwd`
parsets=parsets
logs=logs
slurms=slurmFiles
mkdir -p $parsets
mkdir -p $logs
mkdir -p $slurms

####################
# Control flags

# Primary calibration
DO_1934_CAL=true
DO_FLAG_1934=true
DO_FIND_BANDPASS=true

# Calibration & imaging of the 'science' field.
DO_SCIENCE_FIELD=true
DO_FLAG_SCIENCE=true
DO_APPLY_BANDPASS=true
DO_AVERAGE_CHANNELS=true
DO_CONT_IMAGING=true
DO_SELFCAL=false
DO_SPECTRAL_IMAGING=false
DO_MOSAIC=true

####################
# Input Scheduling Blocks (SBs)
# Location of the SBs
DIR_SB=/scratch2/askap/askapops/archive/askap-scheduling-blocks
# SB with 1934-638 observation
SB_1934="SET_THIS"
MS_INPUT_1934=""
# SB with science observation
SB_SCIENCE="SET_THIS"
MS_INPUT_SCIENCE=""

####################
# Which beams to use.
BEAM_MIN=0
BEAM_MAX=8
nbeam=9


####################
##  BANDPASS CAL

# Base name for the 1934 measurement sets after splitting
MS_BASE_1934=1934_beamBEAM.ms
# Number of channels
NUM_CHAN=16416
# Channel range for splitting
CHAN_RANGE_1934="1-16416"
# Location of 1934-638, formatted for use in cbpcalibrator
DIRECTION_1934="[19h39m25.036, -63.42.45.63, J2000]"
# Name of the table for the bandpass calibration parameters
TABLE_BANDPASS=calparameters_1934_bp.tab
# Number of cycles used in cbpcalibrator
NCYCLES_BANDPASS_CAL=25

# Dynamic threshold applied to amplitudes [sigma]
FLAG_THRESHOLD_DYNAMIC_1934=4.0
# Second amplitude threshold applied [hardware units - before calibration]
FLAG_THRESHOLD_AMPLITUDE_1934=0.2

####################
## Science observation

# This allows selection of particular scans from the science
# observation. If this isn't needed, leave as a blank string.
SCAN_SELECTION_SCIENCE=""
# Base name for the science observation measurement set
MS_BASE_SCIENCE=scienceObservation.ms
# Direction of the science field
directionSci=""

# Range of channels in science observation (used in splitting and averaging)
CHAN_RANGE_SCIENCE="1-16416"
# Number of channels in science observation (used in applying the bandpass solution)
NUM_CHAN=16416
# Number of channels to be averaged to create continuum measurement set
NUM_CHAN_TO_AVERAGE=54

# Dynamic threshold applied to amplitudes [sigma]
FLAG_THRESHOLD_DYNAMIC_SCIENCE=4.0
# Second amplitude threshold applied [hardware units - before calibration]
FLAG_THRESHOLD_AMPLITUDE_SCIENCE=0.2

# Number of Taylor terms to create in MFS imaging
NUM_TAYLOR_TERMS=3
# Number of CPUs to use on each core in the continuum imaging
CPUS_PER_CORE_CONT_IMAGING=20

# base name for images: if IMAGE_BASE_CONT=i.blah then we'll get
# image.i.blah, image.i.blah.restored, psf.i.blah etc
IMAGE_BASE_CONT=i.cont
# number of pixels on the side of the images to be created
NUM_PIXELS_CONT=4096
# Size of the pixels in arcsec
CELLSIZE_CONT=10
# Frequency at which continuum image is made [Hz]
MFS_REF_FREQ=""
# Restoring beam: 'fit' will fit the PSF to determine the appropriate
# beam, else give a size
RESTORING_BEAM_CONT=fit

####################
# Gridding parameters for continuum imaging
GRIDDER_SNAPSHOT_IMAGING=true
GRIDDER_SNAPSHOT_WTOL=2600
GRIDDER_WMAX=2600
GRIDDER_NWPLANES=99
GRIDDER_OVERSAMPLE=4
GRIDDER_MAXSUPPORT=512

####################
# Cleaning parameters for continuum imaging
CLEAN_ALGORITHM=BasisfunctionMFS
CLEAN_MINORCYCLE_NITER=500
CLEAN_GAIN=0.5
CLEAN_SCALES="[0,3,10]"
CLEAN_THRESHOLD_MINORCYCLE="[30%, 0.9mJy]"
CLEAN_THRESHOLD_MAJORCYCLE=1mJy
CLEAN_NUM_MAJORCYCLES=2

####################
# Parameters for preconditioning (A.K.A. weighting)
PRECONDITIONER_LIST="[Wiener, GaussianTaper]"
PRECONDITIONER_GAUSS_TAPER="[30arcsec, 30arcsec, 0deg]"
PRECONDITIONER_WIENER_ROBUSTNESS=0.5
PRECONDITIONER_WIENER_TAPER=""

####################
# Self-calibration parameters
# Interval [sec] over which to solve for self-calibration
SELFCAL_INTERVAL=10
# Number of loops of self-calibration
SELFCAL_NUM_LOOPS=5
# Should we keep the images from the intermediate selfcal loops?
SELFCAL_KEEP_IMAGES=true
# SNR threshold for detection with selavy in determining selfcal sources
SELFCAL_SELAVY_THRESHOLD=15
# Division of image for source-finding in selfcal
SELFCAL_SELAVY_NSUBX=6
SELFCAL_SELAVY_NSUBY=3

# base name for images: if IMAGE_BASE_SPECTRAL=i.blah then we'll get
# image.i.blah, image.i.blah.restored, psf.i.blah etc
IMAGE_BASE_SPECTRAL=i.spectral
# number of pixels on the side of the images to be created
NUM_PIXELS_SPECTRAL=2048
# Size of the pixels in arcsec
CELLSIZE_SPECTRAL=10
# Frequency range for the spectral imager [Hz]
FREQ_RANGE_SPECTRAL="713.e6,1013.e6"

# Beam arrangement, used in linmos. If one of "diamond",
# "octagon",... then the positions are filled automatically.
# The name of the beam footprint. This needs to be recognised by footprint.py - see beamArrangements.sh
BEAM_FOOTPRINT_NAME="diamond"
# The position angle of the beam footprint
BEAM_FOOTPRINT_PA=0
# This is the set of beam offsets used by linmos. This can be set manually instead of getting them from footprint.py
LINMOS_BEAM_OFFSETS=""
# Which frequency band are we in - determines beam arrangement (1,2,3,4 - 1 is lowest frequency)
FREQ_BAND_NUMBER=1
# Scale factor for beam arrangement, in format like '1deg'. Do not change if using the footprint.py names.
LINMOS_BEAM_SPACING="1deg"
# Reference beam for PSF
LINMOS_PSF_REF=0
