##############################################################################
# Continuum Cube Imaging (Clean)
##############################################################################

imagebase=i.cube.clean

cat > cimager-cont-cube-clean.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=6GB:mpiprocs=1
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N contcube-clean
#PBS -m a
#PBS -j oe

###########
# To run:
# qsub -J 1-304 cimager-cont-cube-clean.qsub
#
###########

cd \${PBS_O_WORKDIR}

imageName="image.${imagebase}_ch\${PBS_ARRAY_INDEX}"
ms=MS/coarse_chan_\`expr \${PBS_ARRAY_INDEX} - 1\`.ms

parset=config/cimager-cont-cube-clean-\${PBS_JOBID}.in
cat > \$parset << EOF_INNER
Cimager.dataset                                 = \$ms

Cimager.Images.Names                            = [\${imageName}]
Cimager.Images.shape                            = [3328,3328]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.\${imageName}.frequency           = [1.420e9,1.420e9]
Cimager.Images.\${imageName}.nchan               = 1
Cimager.Images.\${imageName}.direction           = [12h30m00.00, -45.00.00.00, J2000]
Cimager.Images.\${imageName}.nterms              = 1
#Cimager.Images.writeAtMajorCycle                = true
#
#Cimager.visweights                              = MFS
#Cimager.visweights.MFS.reffreq                  = 1.420e9
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 2000
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
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 1000
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = [0, 3, 10, 30]
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.psfwidth                   = 1024
Cimager.solver.Clean.logevery                   = 100
Cimager.threshold.minorcycle                    = [10%, 10mJy]
Cimager.threshold.majorcycle                    = 20mJy
Cimager.ncycles                                 = 3
#
Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 100
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

log=log/cimager-cont-cube-clean-\${PBS_JOBID}.log

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs \$parset > \$log

EOF

if [ "${DRYRUN}" == "false" ]; then
    echo "Continuum Cube Imager (Clean): Submitting task"

    # Add dependencies
    unset DEPENDS
    if [ "${QSUB_CAL}" ] || [ "${QSUB_MSSPLIT}" ]; then
        if [ "${QSUB_CAL}" ]; then
            DEPENDS="-W depend=afterok:${QSUB_CAL}"
        else
            DEPENDS="-W depend=afterok:${QSUB_MSSPLIT}"
        fi
    fi

    # Submit the jobs
    if [ "${DEPENDS}" ]; then
        QSUB_CONTCUBECLEAN=`${QSUB_CMD} ${DEPENDS} -J1-304 cimager-cont-cube-clean.qsub`
    else
        QSUB_CONTCUBECLEAN=`${QSUB_CMD} -J1-304 cimager-cont-cube-clean.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTCUBECLEAN}"
    fi
    unset DEPENDS
    DEPENDS="-W depend=afterok:${QSUB_CONTCUBECLEAN}"

### Run makecube using the make-spectral-cube.qsub script

    DODELETE=true
    NUMCH=304

    ISRESTORED=true
    IMAGEBASE=image.${imagebase}
    OUTPUT=image.${imagebase}.restored
    QSUB_MCREST=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`

    ISRESTORED=false
    IMAGEBASE=image.${imagebase}
    OUTPUT=image.${imagebase}
    QSUB_MCIM=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    
    IMAGEBASE=psf.${imagebase}
    OUTPUT=psf.${imagebase}
    QSUB_MCPSF=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    
    IMAGEBASE=psf.image.${imagebase}
    OUTPUT=psf.image.${imagebase}
    QSUB_MCPSFIM=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    
    IMAGEBASE=mask.${imagebase}
    OUTPUT=mask.${imagebase}
    QSUB_MCMASK=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    
    IMAGEBASE=residual.${imagebase}
    OUTPUT=residual.${imagebase}
    QSUB_MCRESID=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    
    IMAGEBASE=sensitivity.${imagebase}
    OUTPUT=sensitivity.${imagebase}
    QSUB_MCSENS=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    
    IMAGEBASE=weights.${imagebase}
    OUTPUT=weights.${imagebase}
    QSUB_MCWGTS=`${QSUB_CMD} ${DEPENDS} make-spectral-cube.qsub`
    

else
    echo "Continuum Cube Imager (Clean): Dry Run Only"
fi


