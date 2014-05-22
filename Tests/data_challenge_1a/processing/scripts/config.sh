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
QUEUEGROUP=magnusea02

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

# original BETA specs: 304MHz, 36beams
. ${SCRIPTDIR}/config_full_BETA.sh

## trimmed-down BETA: 152MHz, 9beams
#. ${SCRIPTDIR}/config_small_BETA.sh

## ASKAP-12 early science simulation
#. ${SCRIPTDIR}/config_earlyScience_ASKAP12.sh
#. ${SCRIPTDIR}/config_earlyScience_ASKAP12_dec30.sh
#. ${SCRIPTDIR}/config_earlyScience_ASKAP12_dec04.sh

## BETA observation of standard test field J1600-79
#. ${SCRIPTDIR}/config_BETAtestfield.sh
