#!/usr/bin/env bash
##############################################################################
# Calibrate gains in the averaged measurement set
##############################################################################

CALOUTPUT=calparameters.tab

cat > ccalibrator.qsub << EOF
#!/bin/bash
##PBS -W group_list=${QUEUEGROUP}
#PBS -l mppwidth=${GAINS_CAL_MPPWIDTH}
#PBS -l mppnppn=${GAINS_CAL_MPPNPPN}
#PBS -l walltime=06:00:00
##PBS -M first.last@csiro.au
#PBS -N ccalibrator
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/ccalibrator.in << EOF_INNER
Ccalibrator.dataset                              = MS/coarse_chan.ms
Ccalibrator.refgain                              = gain.g11.0.0
Ccalibrator.nAnt                                 = 6
Ccalibrator.nBeam                                = ${NUM_BEAMS_GAINSCAL}
Ccalibrator.solve                                = gains

# Each worker will read a single channel selection
Ccalibrator.Channels                             = [1, %w]

# Output type/filename
Ccalibrator.calibaccess                          = table
Ccalibrator.calibaccess.table                    = ${CALOUTPUT}
Ccalibrator.calibaccess.table.maxant             = 6
Ccalibrator.calibaccess.table.maxbeam            = 36
#Ccalibrator.calibaccess                          = parset
#Ccalibrator.calibaccess.parset                   = result.dat

# Skymodel
Ccalibrator.sources.names                        = skymodel
Ccalibrator.sources.skymodel.direction           = [12h30m00.000, -45.00.00.000, J2000]
Ccalibrator.sources.skymodel.model               = skymodel.image
Ccalibrator.sources.skymodel.nterms              = 3

# Gridder config
Ccalibrator.gridder.snapshotimaging              = true
Ccalibrator.gridder.snapshotimaging.wtolerance   = 1000
Ccalibrator.gridder                              = AWProject
Ccalibrator.gridder.AWProject.wmax               = 1000
Ccalibrator.gridder.AWProject.nwplanes           = 129
Ccalibrator.gridder.AWProject.oversample         = 4
Ccalibrator.gridder.AWProject.diameter           = 12m
Ccalibrator.gridder.AWProject.blockage           = 2m
Ccalibrator.gridder.AWProject.maxfeeds           = 36
Ccalibrator.gridder.AWProject.maxsupport         = 512
Ccalibrator.gridder.AWProject.frequencydependent = true

Ccalibrator.ncycles                              = 5
Ccalibrator.interval                             = 1800
EOF_INNER

aprun -n ${GAINS_CAL_MPPWIDTH} -N ${GAINS_CAL_MPPNPPN} \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh -c ${CONFIGDIR}/ccalibrator.in > ${LOGDIR}/ccalibrator-\${PBS_JOBID}.log
EOF

if [ ! -e ${CALOUTPUT} ]; then
    echo "Calibration: Submitting"

    unset DEPENDS
    if [ "${QSUB_CMODEL}" ] && [ "${QSUB_MSSPLIT}" ]; then
        DEPENDS="afterok:${QSUB_CMODEL},afterok:${QSUB_MSSPLIT}"
        QSUB_CAL=`qsubmit ccalibrator.qsub`
    elif [ "${QSUB_CMODEL}" ]; then
        DEPENDS="afterok:${QSUB_CMODEL}"
        QSUB_CAL=`qsubmit ccalibrator.qsub`
    elif [ "${QSUB_MSSPLIT}" ]; then
        DEPENDS="afterok:${QSUB_MSSPLIT}"
        QSUB_CAL=`qsubmit ccalibrator.qsub`
    else
        QSUB_CAL=`qsubmit ccalibrator.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CAL}"
    fi
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CAL}"
else
    echo "Calibration: Skipping - Output already exists"
fi
