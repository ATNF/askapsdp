##############################################################################
# Calibrate gains in the averaged measurement set
##############################################################################

CALOUTPUT=calparameters.tab

cat > ccalibrator.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=5:mem=23GB:mpiprocs=5+50:ncpus=6:mem=23GB:mpiprocs=6
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

# Skymodel
Ccalibrator.sources.names                        = skymodel
Ccalibrator.sources.skymodel.direction           = [12h30m00.000, -45.00.00.000, J2000]
Ccalibrator.sources.skymodel.model               = skymodel.image
Ccalibrator.sources.skymodel.nterms              = 3

# Gridder config
Ccalibrator.gridder                              = AWProject
Ccalibrator.gridder.AWProject.wmax               = 3500
Ccalibrator.gridder.AWProject.nwplanes           = 5
Ccalibrator.gridder.AWProject.oversample         = 4
Ccalibrator.gridder.AWProject.diameter           = 12m
Ccalibrator.gridder.AWProject.blockage           = 2m
Ccalibrator.gridder.AWProject.maxfeeds           = 36
Ccalibrator.gridder.AWProject.maxsupport         = 2048
Ccalibrator.gridder.AWProject.frequencydependent = true

Ccalibrator.ncycles                              = 5
Ccalibrator.interval                             = 10800s
EOF_INNER

mpirun --mca btl ^openib --mca mtl ^psm \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/ccalibrator.sh -inputs ${CONFIGDIR}/ccalibrator.in > ${LOGDIR}/ccalibrator.log
EOF

if [ "${DRYRUN}" == "false" ]; then
    if [ ! -e ${CALOUTPUT} ]; then
        echo "Calibration: Submitting Task"
        if [ ! "${QSUB_CMODEL}" ] && [ ! "${QSUB_MSSPLIT}" ]; then
            QSUB_CCAL=`${QSUB_CMD} ccalibrator.qsub`
            QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CCAL}"
        else
            QSUB_CCAL=`${QSUB_CMD} -W depend=afterok:${QSUB_CMODEL},afterok:${QSUB_MSSPLIT} ccalibrator.qsub`
        fi
    else
        echo "Calibration: Skipping task - Output already exists"
    fi
else
    echo "Calibration: Dry Run Only"
fi
