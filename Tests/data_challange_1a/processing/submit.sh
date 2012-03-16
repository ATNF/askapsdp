##############################################################################
# General initial steps
##############################################################################

SCRIPTDIR=`pwd`/scripts

# Source the configuration data
. ${SCRIPTDIR}/config.sh

# Make and cd to the workdir
if [ ! -d ${WORKDIR} ]; then
    mkdir ${WORKDIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create workdir"
        exit 1
    fi
fi
cd ${WORKDIR}
if [ $? -ne 0 ]; then
    echo "Error: Failed to CD to workdir"
    exit 1
fi

# Set the qsub alias based
if [ ${BATCH_QUEUE} ]; then
    QSUB_CMD="qsub -q ${BATCH_QUEUE}"
else
    QSUB_CMD="qsub"
fi

# Verify the input measurement set exists
if [ ! -e ${INPUT_MS} ]; then
    echo "Error: Input measurement set does not exist"
    exit 1
fi

# Verify the input sky model exists
if [ ! -e ${INPUT_SKYMODEL} ]; then
    echo "Error: Input sky model does not exist"
    exit 1
fi

# Empty the list of job ids with no deps
unset QSUB_NODEPS

# Make the log directory
LOGDIR=log
if [ ! -d ${LOGDIR} ]; then
    mkdir ${LOGDIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create logdir"
        exit 1
    fi
fi

# Make the config directory (to hold the generated configuration files)
CONFIGDIR=config
if [ ! -d ${CONFIGDIR} ]; then
    mkdir ${CONFIGDIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create configdir"
        exit 1
    fi
fi

# Write a log4cxx config into the work directory
cat > askap.log_cfg << EOF
# Configure the rootLogger
log4j.rootLogger=INFO,STDOUT

log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender
log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout
log4j.appender.STDOUT.layout.ConversionPattern=%-5p %c{2} (%X{mpirank}, %X{hostname}) [%d] - %m%n
EOF

##############################################################################
# Enqueue the processing tasks
##############################################################################

# These jobs are executed if their outputs do not exist
. ${SCRIPTDIR}/generate-sky-model.sh
. ${SCRIPTDIR}/create-coarse-ms.sh
. ${SCRIPTDIR}/gains-calibration.sh

# These jobs are executed per the preference in config.sh
if [ $DO_CONTINUUM_DIRTY == "true" ]; then
. ${SCRIPTDIR}/imager-cont-clean.sh
fi

if [ $DO_CONTINUUM_CLEAN == "true" ]; then
. ${SCRIPTDIR}/imager-cont-dirty.sh
fi

if [ $DO_SPECTRAL_LINE == "true" ]; then
. ${SCRIPTDIR}/imager-spectral-line.sh
fi

##############################################################################
# Execute!!
##############################################################################

# Now all batch jobs have been submitted, unhold the first one. All jobs were
# created with a "hold" so as dependencies could be established without fear
# of a dependency running and finishing before the dependency could be
# established
if [ ! ${DRYRUN} ] && [ "${QSUB_NODEPS}" ]; then
    qrls ${QSUB_NODEPS}
fi
