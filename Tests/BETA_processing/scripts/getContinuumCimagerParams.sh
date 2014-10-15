#!/usr/bin/env bash
#
# Defines the majority of the parameter set used by Cimager, based on
# the user-defined parameters. This script is called by the two
# continuum imaging scripts to help fill out the parset. Upon return,
# the following environment variables are defined:
#   * cimagerParams - the Cimager parset, excluding any calibrate
#   parameters
#   * imageBase - the base name for the image products, incorporating
#   the current beam
#   * directionDefinition - the direction of the centre of the image, 
#   as a Cimager parameter if 'directionSci' is defined
#   * shapeDefinition - the shape of the images, as a Cimager
#   parameter if 'NUM_PIXELS_CONT' is defined
#   * cellsizeDefinition - the shape of the images, as a Cimager
#   parameter if 'CELLSIZE_CONT' is defined
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

imageBase=${IMAGE_BASE_CONT}.beam${BEAM}

# Define the shape parameter, or leave to "advise"
shapeDefinition="# Leave shape definition to Cimager to determine from the data"
if [ "${NUM_PIXELS_CONT}" != "" ] && [ $NUM_PIXELS_CONT -gt 0 ]; then
    shapeDefinition="Cimager.Images.shape                            = [${NUM_PIXELS_CONT}, ${NUM_PIXELS_CONT}]"
fi

# Define the cellsize parameter, or leave to "advise"
cellsizeDefinition="# Leave cellsize definition to Cimager to determine from the data"
if [ "${CELLSIZE_CONT}" != "" ] && [ $CELLSIZE_CONT -gt 0 ]; then
    cellsizeDefinition="Cimager.Images.cellsize                         = [${CELLSIZE_CONT}arcsec, ${CELLSIZE_CONT}arcsec]"
fi

# Define the direction parameter, or leave to "advise"
directionDefinition="# Leave direction definition to Cimager to determine from the data"
if [ "${directionSci}" != "" ]; then
    directionDefinition="Cimager.Images.image.${imageBase}.direction    = ${directionSci}"
fi

# Define the preconditioning
preconditioning="Cimager.preconditioner.Names                    = ${PRECONDITIONER_LIST}"
if [ "`echo ${PRECONDITIONER_LIST} | grep GaussianTaper`" != "" ]; then
    preconditioning="$preconditioning
Cimager.preconditioner.GaussianTaper            = ${PRECONDITIONER_GAUSS_TAPER}"
fi
if [ "`echo ${PRECONDITIONER_LIST} | grep Wiener`" != "" ]; then
    if [ "${PRECONDITIONER_WIENER_ROBUSTNESS}" != "" ]; then
	preconditioning="$preconditioning
Cimager.preconditioner.Wiener.robustness        = ${PRECONDITIONER_WIENER_ROBUSTNESS}"
    fi
    if [ "${PRECONDITIONER_WIENER_TAPER}" != "" ]; then
	preconditioning="$preconditioning
Cimager.preconditioner.Wiener.taper             = ${PRECONDITIONER_WIENER_TAPER}"
    fi
fi

#Define the MFS parameters: visweights and reffreq, or leave to advise
mfsParams="# The following are needed for MFS clean
# This one defines the number of Taylor terms
Cimager.Images.image.${imageBase}.nterms       = ${NUM_TAYLOR_TERMS}
# This one assigns one worker for each of the Taylor terms
Cimager.nworkergroups                           = ${nworkergroupsSci}
# Leave 'Cimager.visweights' to be determined by Cimager, based on nterms"

if [ "${MFS_REF_FREQ}" == "" ] || [ $MFS_REF_FREQ -le 0 ]; then
    mfsParams="${mfsParams}
# Leave 'Cimager.visweights.MFS.reffreq' to be determined by Cimager"
else
    mfsParams="${mfsParams}
# This is the reference frequency - it should lie in your frequency range (ideally in the middle)
Cimager.visweights.MFS.reffreq                  = ${MFS_REF_FREQ}"
fi


cimagerParams="#Standard Parameter set for Cimager
Cimager.dataset                                 = ${msSciAv}
#
# Each worker will read a single channel selection
Cimager.Channels                                = [1, %w]
#
Cimager.Images.Names                            = [image.${imageBase}]
${shapeDefinition}
${cellsizeDefinition}
${directionDefinition}
# This is how many channels to write to the image - just a single one for continuum
Cimager.Images.image.${imageBase}.nchan        = 1
#
${mfsParams}
#
# This defines the parameters for the gridding.
Cimager.gridder.snapshotimaging                 = ${GRIDDER_SNAPSHOT_IMAGING}
Cimager.gridder.snapshotimaging.wtolerance      = ${GRIDDER_SNAPSHOT_WTOL}
Cimager.gridder                                 = WProject
Cimager.gridder.WProject.wmax                   = ${GRIDDER_WMAX}
Cimager.gridder.WProject.nwplanes               = ${GRIDDER_NWPLANES}
Cimager.gridder.WProject.oversample             = ${GRIDDER_OVERSAMPLE}
Cimager.gridder.WProject.maxsupport             = ${GRIDDER_MAXSUPPORT}
Cimager.gridder.WProject.variablesupport        = true
Cimager.gridder.WProject.offsetsupport          = true
#
# These parameters define the clean algorithm 
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = ${CLEAN_ALGORITHM}
Cimager.solver.Clean.niter                      = ${CLEAN_MINORCYCLE_NITER}
Cimager.solver.Clean.gain                       = ${CLEAN_GAIN}
Cimager.solver.Clean.scales                     = ${CLEAN_SCALES}
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.tolerance                  = 0.01
Cimager.solver.Clean.weightcutoff               = zero
Cimager.solver.Clean.weightcutoff.clean         = false
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 50
Cimager.threshold.minorcycle                    = ${CLEAN_THRESHOLD_MINORCYCLE}
Cimager.threshold.majorcycle                    = ${CLEAN_THRESHOLD_MAJORCYCLE}
Cimager.ncycles                                 = ${CLEAN_NUM_MAJORCYCLES}
Cimager.Images.writeAtMajorCycle                = false
#
${preconditioning}
#
Cimager.restore                                 = true
Cimager.restore.beam                            = ${RESTORING_BEAM_CONT}
"
