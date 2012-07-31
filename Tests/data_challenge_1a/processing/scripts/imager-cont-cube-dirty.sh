#!/usr/bin/env bash
##############################################################################
# Continuum Cube Imaging (Dirty)
##############################################################################

imagebase=i.cube.dirty

cat > cimager-cont-cube-dirty.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=6GB:mpiprocs=1
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N contcube-dirty
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT

###########
# To run:
# qsub -J 0-303 cimager-cont-cube-dirty.qsub
#
###########

cd \${PBS_O_WORKDIR}

imageName="image.${imagebase}_ch\${PBS_ARRAY_INDEX}"
ms=MS/coarse_chan_\${PBS_ARRAY_INDEX}.ms

basefreq=1.420e9
dfreq=1.e6
freq=\`echo \$basefreq \$dfreq \${PBS_ARRAY_INDEX} | awk '{printf "%8.6e",\$1-\$2*\$3}'\`

parset=config/cimager-cont-cube-dirty-\${PBS_JOBID}.in
cat > \$parset << EOF_INNER
Cimager.dataset                                 = \$ms

Cimager.Images.Names                            = [\${imageName}]
Cimager.Images.shape                            = [3328,3328]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.\${imageName}.frequency           = [\${freq},\${freq}]
Cimager.Images.\${imageName}.nchan               = 1
Cimager.Images.\${imageName}.direction           = [12h30m00.00, -45.00.00.00, J2000]
Cimager.Images.\${imageName}.nterms              = 1
#Cimager.Images.writeAtMajorCycle                = true
#
#Cimager.visweights                              = MFS
#Cimager.visweights.MFS.reffreq                  = 1.420e9
#
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 3500
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
#
Cimager.preconditioner.Names                    = None
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = True
#
# Apply calibration
Cimager.calibrate                               = false
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = 
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

log=log/cimager-cont-cube-dirty-\${PBS_JOBID}.log

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs \$parset > \$log
EOF

# Submit job
echo "Continuum Cube Imager (Dirty): Submitting"

unset DEPENDS
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
elif [ "${QSUB_MSSPLIT}" ]; then
    DEPENDS="afterok:${QSUB_MSSPLIT}"
fi

QSUB_CONTCUBEDIRTY=`qsubmit -J 0-303 cimager-cont-cube-dirty.qsub`
GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CONTCUBEDIRTY}"

if [ ! "${DEPENDS}" ]; then
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTCUBEDIRTY}"
fi

# This becomes a dependency for the makecube jobs
DEPENDS="afterok:${QSUB_CONTCUBEDIRTY}"

# Run makecube using the make-spectral-cube.qsub script
DODELETE=true
FIRSTCH=0
FINALCH=303

IMAGEPREFIX="image.${imagebase}_ch"
IMAGESUFFIX=".restored"
OUTPUTCUBE=image.${imagebase}.restored
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGESUFFIX=""
IMAGEPREFIX="psf.${imagebase}_ch"
OUTPUTCUBE=psf.${imagebase}
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="sensitivity.${imagebase}_ch"
OUTPUTCUBE=sensitivity.${imagebase}
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="weights.${imagebase}_ch"
OUTPUTCUBE=weights.${imagebase}
. ${SCRIPTDIR}/make-spectral-cube.sh
