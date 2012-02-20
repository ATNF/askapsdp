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
BATCH_QUEUE=routequeue

##############################################################################
# General initial steps
##############################################################################

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
# Measurement Set Averaging (For Gains calibration and Continuum imaging
##############################################################################

# Create a directory to hold the output measurement sets
AVGMSDIR=MS
if [ ! -d ${AVGMSDIR} ]; then
    mkdir ${AVGMSDIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${AVGDIR}"
        exit 1
    fi
fi

# Create the qsub file
cat > split_coarse.qsub << EOF
#!/bin/bash
#PBS -q ${BATCH_QUEUE}
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
#PBS -l walltime=06:00:00
##PBS -M first.last@csiro.au
#PBS -N split
#PBS -m a
#PBS -j oe

#######
# TO RUN (300 jobs):
#  qsub -J 0-299 split.qsub
#######

cd \${PBS_O_WORKDIR}

WIDTH=54
STARTCHAN=1
ENDCHAN=16200

RANGE1=\`expr \${PBS_ARRAY_INDEX} \* \${WIDTH} + \${STARTCHAN}\`
RANGE2=\`expr \${RANGE1} + \${WIDTH} - 1\`

cat > ${CONFIGDIR}/mssplit_coarse_\${PBS_ARRAY_INDEX}.in << EOF_INNER
# Input measurement set
# Default: <no default>
vis         = ${INPUT_MS}

# Output measurement set
# Default: <no default>
outputvis   = MS/coarse_chan_\${PBS_ARRAY_INDEX}.ms

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based. 
# Default: <no default>
channel     = \${RANGE1}-\${RANGE2}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = \${WIDTH}
EOF_INNER

\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/mssplit.sh -inputs ${CONFIGDIR}/mssplit_coarse_\${PBS_ARRAY_INDEX}.in > ${LOGDIR}/mssplit_coarse_\${PBS_ARRAY_INDEX}.log
EOF

if [ ! -e MS/coarse_chan_0.ms ]; then
    echo "MS Split and Average: Submitting task"
    QSUB_MSSPLIT=`qsub -h -J 0-299 split_coarse.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_MSSPLIT}"
else
    echo "MS Split and Average: Skipping task - Output already exists"
fi

##############################################################################
# Sky Model Image Generation
##############################################################################

SKYMODEL_OUTPUT=skymodel.image.taylor

cat > cmodel.qsub << EOF
#!/bin/bash
#PBS -q ${BATCH_QUEUE}
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=2:ncpus=12:mem=23GB:mpiprocs=12
#PBS -l walltime=00:15:00
##PBS -M first.last@csiro.au
#PBS -N cmodel
#PBS -m a
#PBS -j oe
#PBS -r n

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/cmodel.in << EOF_INNER
# The below specifies the GSM source is a duchamp output file
Cmodel.gsm.database       = duchamp
Cmodel.gsm.file           = ${INPUT_SKYMODEL}
Cmodel.gsm.ref_freq       = 1.421GHz

# General parameters
Cmodel.bunit              = Jy/pixel
Cmodel.frequency          = 1.421GHz
Cmodel.increment          = 300MHz
Cmodel.flux_limit         = 1mJy
Cmodel.shape              = [3560, 3560]
Cmodel.cellsize           = [9.1234arcsec, 9.1234arcsec]
Cmodel.direction          = [12h30m00.00, -45.00.00.00, J2000]
Cmodel.stokes             = [I]
Cmodel.nterms             = 3
Cmodel.batchsize          = 10

# Output specific parameters
Cmodel.output             = casa
Cmodel.filename           = ${SKYMODEL_OUTPUT}
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/cmodel.sh -inputs ${CONFIGDIR}/cmodel.in > ${LOGDIR}/cmodel.log
EOF

if [ ! -e ${SKYMODEL_OUTPUT}.0 ]; then
    echo "Sky Model Image: Submitting task"
    QSUB_CMODEL=`qsub -h cmodel.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CMODEL}"
else
    echo "Sky Model Image: Skipping task - Output already exists"
fi

##############################################################################
# Calibrate gains in the averaged measurement set
##############################################################################

CALOUTPUT=calparameters.tab

cat > ccalibrator.qsub << EOF
#!/bin/bash
#PBS -q ${BATCH_QUEUE}
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=23GB:mpiprocs=1+25:ncpus=12:mem=23GB:mpiprocs=12
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N ccalibrator
#PBS -m a
#PBS -j oe

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/ccalibrator.in << EOF_INNER
Ccalibrator.dataset                              = MS/coarse_chan_%w.ms
Ccalibrator.refgain                              = gain.g11.0.0
Ccalibrator.nAnt                                 = 6
Ccalibrator.nBeam                                = 36
Ccalibrator.solve                                = gains

# Output type/filename
Ccalibrator.calibaccess                          = table
Ccalibrator.calibaccess.table                    = ${CALOUTPUT}
Ccalibrator.calibaccess.table.maxant             = 6
Ccalibrator.calibaccess.table.maxbeam            = 36
Ccalibrator.calibaccess.table.maxchan            = 300

# Skymodel
Ccalibrator.sources.names                        = skymodel
Ccalibrator.sources.skymodel.direction           = [12h30m00.000, -45.00.00.000, J2000]
Ccalibrator.sources.skymodel.model               = skymodel.image
Ccalibrator.sources.skymodel.nterms              = 3

# Gridder config
Ccalibrator.gridder                              = AWProject
Ccalibrator.gridder.AWProject.wmax               = 5000
Ccalibrator.gridder.AWProject.nwplanes           = 1
Ccalibrator.gridder.AWProject.oversample         = 4
Ccalibrator.gridder.AWProject.diameter           = 12m
Ccalibrator.gridder.AWProject.blockage           = 2m
Ccalibrator.gridder.AWProject.maxfeeds           = 36
Ccalibrator.gridder.AWProject.maxsupport         = 512
Ccalibrator.gridder.AWProject.frequencydependent = false

Ccalibrator.ncycles                              = 5
Ccalibrator.interval                             = 3600s
EOF_INNER

mpirun --mca btl ^openib --mca mtl ^psm \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh -inputs ${CONFIGDIR}/ccalibrator.in > ${LOGDIR}/ccalibrator.log
EOF

if [ ! -e ${CALOUTPUT} ]; then
    echo "Calibration: Submitting Task"
    if [ ! ${QSUB_CMODEL} ] && [ ! ${QSUB_MSSPLIT} ]; then
        QSUB_CCAL=`qsub ccalibrator.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CCAL}"
    else 
        QSUB_CCAL=`qsub -W depend=afterok:${QSUB_CMODEL},afterok:${QSUB_MSSPLIT} ccalibrator.qsub`
    fi
else
    echo "Calibration: Skipping task - Output exists"
fi

##############################################################################
# Continuum Imaging
##############################################################################

cat > cimager-cont.qsub << EOF
#!/bin/bash
#PBS -q ${BATCH_QUEUE}
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=23GB:mpiprocs=1+50:ncpus=6:mem=23GB:mpiprocs=6
#PBS -l walltime=02:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-img
#PBS -m a
#PBS -j oe

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/cimager-cont.in << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan_%w.ms

Cimager.Images.Names                            = [image.i.dirty]
Cimager.Images.shape                            = [3560,3560]
Cimager.Images.cellsize                         = [9.1234arcsec, 9.1234arcsec]
Cimager.Images.image.i.dirty.frequency          = [1.420e9,1.420e9]
Cimager.Images.image.i.dirty.nchan              = 1
Cimager.Images.image.i.dirty.direction          = [12h30m00.00, -45.00.00.00, J2000]
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 3000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 3000
Cimager.gridder.AWProject.nwplanes              = 5
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 2048
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = None

# Apply calibration
Cimager.calibrate                               = true
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs ${CONFIGDIR}/cimager-cont.in > ${LOGDIR}/cimager-cont.log
EOF

echo "Continuum Imager: Submitting task"
if [ ${QSUB_CCAL} ]; then
    QSUB_CONTIMG=`qsub -W depend=afterok:${QSUB_CCAL} cimager-cont.qsub`
else 
    QSUB_CONTIMG=`qsub cimager-cont.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTIMG}"
fi

##############################################################################
# Execute!!
##############################################################################

# Now all batch jobs have been submitted, unhold the first one. All jobs were
# created with a "hold" so as dependencies could be established without fear
# of a dependency running and finishing before the dependency could be
# established
qrls $QSUB_NODEPS
