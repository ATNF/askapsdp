#
# ASKAP Data Challenge 1A Processing Pipeline
#

##############################################################################
# Configuration
##############################################################################

# A directory will be created to run the pipeline in. This directory can be reused,
# or a new one can be created for each run. Uncomment either of the below WORKDIR
# options to suit.
#WORKDIR=run1
WORKDIR="run_`date +%Y%m%d_%H%M%S`"

# The PBS Group ID to which the job will be billed (eg. astronomy554 on epic)
QUEUEGROUP=astronomy554

# Location (relative to the workdir or absolute) of the input measurement set
INPUT_MS=../input/dc1a.ms

# Location (relative to workdir or absolute) of the input sky model (component list)
INPUT_SKYMODEL_XML=../input/skymodel-duchamp.xml
INPUT_SKYMODEL_TXT=../input/skymodel-duchamp.txt

# If ASKAP_ROOT is not set in your environment, add the path here and uncomment
#ASKAP_ROOT=<path to ASKAPsoft>

# PBS queue to submit jobs to. This is usually the "routequeue" for epic, however
# if a reservation has been made this can be changed to submit into the reservation
# Default: If this is not set, the system default queue will be used.
#BATCH_QUEUE=routequeue

# Dryrun (true or false)
# If this is set true, the jobs will not be enqueued, only configuration
# files, etc. will be created.
DRYRUN=false

# Do calibration (true or false)
# If true, calibration parameters will be determined and applied
# during imaging
DO_CALIBRATION=false

# Do Continuum Imaging - Dirty (true or false)
DO_CONTINUUM_DIRTY=true

# Do Continuum Imaging - Clean (true or false)
DO_CONTINUUM_CLEAN=true

# Do analysis on continuum image
DO_ANALYSIS=true

# Do Continuum-cube Imaging - Dirty (true or false)
DO_CONTINUUM_CUBE_DIRTY=false

# Do Continuum-cube Imaging - Clean (true or false)
DO_CONTINUUM_CUBE_CLEAN=false

# Do Spectral Line Imaging (true or false)
DO_SPECTRAL_LINE=false


#####################################
#
# Some constants used in the scripts, extracted here so that we can
# change between different modes - eg. halve the bandwidth

# final channel, used by create-coarse-ms.sh
END_CHANNEL_CREATECOARSE=16416
# number of workers used by create-coarse-ms.sh
NUM_WORKERS_CREATECOARSE=304
QSUB_RANGE_CREATECOARSE="0-303"

# number of worker nodes needed for gains-calibration.sh - work with 2 worker cpus per node
GAINS_CAL_SELECT="1:ncpus=1:mem=23GB:mpiprocs=1+152:ncpus=2:mem=23GB:mpiprocs=2"

# number of worker nodes needed for imager-cont-clean.sh - work with 2 worker cpus per node
CONT_CLEAN_SELECT="1:ncpus=1:mem=23GB:mpiprocs=1+152:ncpus=2:mem=23GB:mpiprocs=2"

# number of worker nodes needed for imager-cont-dirty.sh - work with 6 worker cpus per node, plus extra on the master's node
CONT_DIRTY_SELECT="1:ncpus=3:mem=23GB:mpiprocs=3+51:ncpus=6:mem=23GB:mpiprocs=6"

# base frequency for continuum cubes
CONT_CUBE_BASEFREQ=1.420e9
# number of workers used for continuum cubes, and qsub range
NUM_WORKERS_CONT_CUBE=303
QSUB_RANGE_CONT_CUBE="0-303"
# final channel used for the make-spectral-cube call for continuum cubes
CONT_CUBE_FINALCH=303

# base frequency for spectral-line cubes
SPECTRAL_CUBE_BASEFREQ=1.420e9
# number of workers used for spectral-line cubes, and qsub range
NUM_WORKERS_SPECTRAL_CUBE=16416
QSUB_RANGE_SPECTRAL_CUBE_1="0-8207"
QSUB_RANGE_SPECTRAL_CUBE_1="8208-16415"
QSUB_RANGE_SPECTRAL_CUBE_FULL="0-16415"
# final channel used for the make-spectral-cube call for continuum cubes
SPECTRAL_CUBE_FINALCH=16415

# reference frequency for sky model
SKYMODEL_REFFREQ="1.421GHz"
SKYMODEL_FREQ="1.421GHz"

