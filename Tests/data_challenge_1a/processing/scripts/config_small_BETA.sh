#
# ASKAP Data Challenge 1A Processing Pipeline
#

##############################################################################
# Specific Configuration
##############################################################################
#
##############
#
# This file defines various constants used in the scripts.
#
# This version is for a simulation of the reduced-spec BETA: half the
# original bandwidth, and only 9 PAF beams

    echo Running pipeline for half-spec BETA : 152MHz BW, 9 beams

#################
# THIS IS FOR HALF BANDWIDTH AND 9 BEAMS
    
# final channel, used by create-coarse-ms.sh
    END_CHANNEL_CREATECOARSE=8208
# number of workers used by create-coarse-ms.sh
    NUM_WORKERS_CREATECOARSE=152
    QSUB_RANGE_CREATECOARSE="0-151"
    
# number of worker nodes needed for gains-calibration.sh - work with 2 worker cpus per node
    GAINS_CAL_SELECT="1:ncpus=1:mem=23GB:mpiprocs=1+76:ncpus=2:mem=23GB:mpiprocs=2"
    NUM_BEAMS_GAINSCAL=9

# image size -- number of pixels and cellsize
    IMAGING_NUM_PIXELS=2048
    IMAGING_CELLSIZE=10arcsec

# number of worker nodes needed for imager-cont-clean.sh - work with 2 worker cpus per node (but with nworkergroups=3)
    CONT_CLEAN_SELECT="1:ncpus=1:mem=23GB:mpiprocs=1+76:ncpus=6:mem=23GB:mpiprocs=6"
    CONT_CLEAN_FREQ=1.270e9

# number of worker nodes needed for imager-cont-dirty.sh - work with 6 worker cpus per node, plus extra on the master's node
    CONT_DIRTY_SELECT="1:ncpus=3:mem=23GB:mpiprocs=3+25:ncpus=6:mem=23GB:mpiprocs=6"
    CONT_DIRTY_FREQ=1.270e9

# base frequency for continuum cubes
    CONT_CUBE_FREQ_ZERO_CHAN=1.345e9
# number of workers used for continuum cubes, and qsub range
    NUM_WORKERS_CONT_CUBE=152
    QSUB_RANGE_CONT_CUBE="0-151"
# final channel used for the make-spectral-cube call for continuum cubes
    CONT_CUBE_FINALCH=151

# base frequency for spectral-line cubes
    SPECTRAL_CUBE_FREQ_ZERO_CHAN=1.345e9
# number of workers used for spectral-line cubes, and qsub range
    NUM_WORKERS_SPECTRAL_CUBE=8208
    QSUB_RANGE_SPECTRAL_CUBE_1="0-4103"
    QSUB_RANGE_SPECTRAL_CUBE_2="4104-8207"
    QSUB_RANGE_SPECTRAL_CUBE_FULL="0-8207"
# final channel used for the make-spectral-cube call for continuum cubes
    SPECTRAL_CUBE_FINALCH=8207

# reference frequency for sky model
    SKYMODEL_REFFREQ="1.270GHz"
    SKYMODEL_FREQ="1.270GHz"
    SKYMODEL_ALT_FREQ="1.270e9"

# subsection of image used for analysis
    ANALYSIS_SUBSECTION="[301:1800,301:1800,*,*]"
    THRESHIMAGE=detectionThreshold.i.clean
    NOISEIMAGE=noiseMap.i.clean
    SNRIMAGE=snr.i.clean
    AVERAGEIMAGE=meanMap.i.clean

