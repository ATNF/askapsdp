#!/usr/bin/env bash

if [ $DO_SPECTRAL_IMAGING == true ]; then

    echo "Imaging the spectral-line science observation"

    imageBase=${IMAGE_BASE_SPECTRAL}.beam${BEAM}

    shapeDefinition="# Leave shape definition to advise"
    if [ $NUM_PIXELS_SPECTRAL -gt 0 ]; then
	shapeDefinition="Cimager.Images.shape                            = [${NUM_PIXELS_SPECTRAL}, ${NUM_PIXELS_SPECTRAL}]"
    fi
    cellsizeDefinition="# Leave cellsize definition to advise"
    if [ $CELLSIZE_SPECTRAL -gt 0 ]; then
	cellsizeDefinition="Simager.Images.cellsize                         = [${CELLSIZE_SPECTRAL}arcsec, ${CELLSIZE_SPECTRAL}arcsec]"
    fi

    sbatchfile=$slurms/science_continuumImage_beam$BEAM.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=4000
#SBATCH --ntasks-per-node=20
#SBATCH --job-name sl-img
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=$parsets/science_spectral_imager_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset << EOF

Simager.dataset                                 = ${msSci}
#
Simager.Images.name                             = image.${imageBase}
${shapeDefinition}
${cellsizeDefinition}
Simager.Images.frequency                        = [${FREQ_RANGE_SPECTRAL}]
Simager.Images.direction                        = ${directionSci}
#
Simager.gridder.snapshotimaging                 = true
Simager.gridder.snapshotimaging.wtolerance      = 2600
Simager.gridder                                 = WProject
Simager.gridder.WProject.wmax                   = 2600
Simager.gridder.WProject.nwplanes               = 99
Simager.gridder.WProject.oversample             = 4
Simager.gridder.WProject.maxsupport             = 512
Simager.gridder.WProject.variablesupport        = true
Simager.gridder.WProject.offsetsupport          = true
#
Simager.solver                                  = Dirty
Simager.solver.Dirty.tolerance                  = 0.1
Simager.solver.Dirty.verbose                    = True
Simager.ncycles                                 = 0
#
#Simager.preconditioner.Names                    = [Wiener, GaussianTaper]
#Simager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Simager.preconditioner.Names                    = [Wiener]
Simager.preconditioner.Wiener.robustness        = 0.25
#Simager.preconditioner.Wiener.taper             = 64

# Do not apply calibration as yet
Simager.calibrate                               = false
EOF

log=$logs/science_spectral_imager_beam${BEAM}_\${SLURM_JOB_ID}.log

# Now run the simager
aprun -B ${simager} -c $parset > $log
ERR=$?
if [ ${ERR} -ne 0 ]; then
    echo "Error: simager returned error code ${ERR}"
    exit 1
fi


EOFOUTER




fi
