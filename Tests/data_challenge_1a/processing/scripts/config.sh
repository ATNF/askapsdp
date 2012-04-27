#
# ASKAP Data Challenge 1A Processing Pipeline
#

##############################################################################
# Configuration
##############################################################################

# A directory will be created to run the pipeline in. This directory can be reused,
# or a new one can be created for each run. Uncomment either of the below WORKDIR
# options to suit.
WORKDIR=run1
#WORKDIR="run_`date +%Y%m%d_%H%M%S`"

# The PBS Group ID to which the job will be billed (eg. astronomy116 on epic)
QUEUEGROUP=astronomy116

# Location (relative to the workdir or absolute) of the input measurement set
INPUT_MS=../input/dc1a.ms

# Location (relative to workdir or absolute) of the input sky model (component list)
INPUT_SKYMODEL=../input/skymodel-duchamp.txt

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

# Do Continuum-cube Imaging - Dirty (true or false)
DO_CONTINUUM_CUBE_DIRTY=true

# Do Continuum-cube Imaging - Clean (true or false)
DO_CONTINUUM_CUBE_CLEAN=true

# Do Spectral Line Imaging (true or false)
DO_SPECTRAL_LINE=false
