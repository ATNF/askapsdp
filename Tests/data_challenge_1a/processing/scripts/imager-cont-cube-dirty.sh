#!/usr/bin/env bash
##############################################################################
# Continuum Cube Imaging (Dirty)
##############################################################################

imagebase=i.cube.dirty

# Create a work dir directory and one to hold the output measurement sets
SL_WORK_DIR=sl-work-dir
if [ ! -d ${SL_WORK_DIR} ]; then
    mkdir ${SL_WORK_DIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${SL_WORK_DIR}"
        exit 1
    fi
    mkdir ${SL_WORK_DIR}/MS
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${SL_WORK_DIR}/MS"
        exit 1
    fi
fi

mv askap.log_cfg ${SL_WORK_DIR}

cat > cimager-cont-cube-dirty.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=6GB:mpiprocs=1
#PBS -l walltime=04:00:00
##PBS -M first.last@csiro.au
#PBS -N contcube-dirty
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

###########
# To run:
# qsub -J ${QSUB_RANGE_CONT_CUBE} cimager-cont-cube-dirty.qsub
#
###########

cd \${PBS_O_WORKDIR}/${SL_WORK_DIR}

imageName="image.${imagebase}_ch\${PBS_ARRAY_INDEX}"
ms=../MS/coarse_chan_\${PBS_ARRAY_INDEX}.ms

basefreq=${CONT_CUBE_FREQ_ZERO_CHAN}
dfreq=1.e6
freq=\`echo \$basefreq \$dfreq \${PBS_ARRAY_INDEX} | awk '{printf "%8.6e",\$1-\$2*\$3}'\`

parset=../config/cimager-cont-cube-dirty-\${PBS_JOBID}.in
cat > \$parset << EOF_INNER
Cimager.dataset                                 = \$ms

Cimager.Images.Names                            = [\${imageName}]
Cimager.Images.shape                            = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Cimager.Images.cellsize                         = [${IMAGING_CELLSIZE},${IMAGING_CELLSIZE}]
Cimager.Images.\${imageName}.frequency           = [\${freq},\${freq}]
Cimager.Images.\${imageName}.nchan               = 1
Cimager.Images.\${imageName}.direction           = ${IMAGING_DIRECTION}
Cimager.Images.\${imageName}.nterms              = 1
#Cimager.Images.writeAtMajorCycle                = true
#
#Cimager.visweights                              = MFS
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = ${IMAGING_WMAX}
Cimager.gridder.AWProject.nwplanes              = 129
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 512
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0
#
Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = ${IMAGING_EQUALISE}
#
# Apply calibration
Cimager.calibrate                               = false
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
#Cimager.calibaccess                             = parset
#Cimager.calibaccess.parset                      = result.dat
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

log=../log/cimager-cont-cube-dirty-\${PBS_JOBID}.log

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -c \$parset > \$log
EOF

# Submit job
echo "Continuum Cube Imager (Dirty): Submitting"

unset DEPENDS
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
elif [ "${QSUB_MSSPLIT}" ]; then
    DEPENDS="afterok:${QSUB_MSSPLIT}"
fi

QSUB_CONTCUBEDIRTY=`qsubmit -J ${QSUB_RANGE_CONT_CUBE} cimager-cont-cube-dirty.qsub`
GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CONTCUBEDIRTY}"

if [ ! "${DEPENDS}" ]; then
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTCUBEDIRTY}"
fi

# This becomes a dependency for the makecube jobs
DEPENDS="afterok:${QSUB_CONTCUBEDIRTY}"

# Run makecube using the make-spectral-cube.qsub script
DODELETE=false
FIRSTCH=0
FINALCH=${CONT_CUBE_FINALCH}
RESTFREQ=-1.

IMAGEPREFIX="${SL_WORK_DIR}/image.${imagebase}_ch"
IMAGESUFFIX=".restored"
OUTPUTCUBE=image.${imagebase}.restored
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGESUFFIX=""
IMAGEPREFIX="${SL_WORK_DIR}/psf.${imagebase}_ch"
OUTPUTCUBE=psf.${imagebase}
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="${SL_WORK_DIR}/sensitivity.${imagebase}_ch"
OUTPUTCUBE=sensitivity.${imagebase}
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="${SL_WORK_DIR}/weights.${imagebase}_ch"
OUTPUTCUBE=weights.${imagebase}
. ${SCRIPTDIR}/make-spectral-cube.sh
