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
#   parameter if 'pixsizeCont' is defined
#   * cellsizeDefinition - the shape of the images, as a Cimager
#   parameter if 'cellsizeCont' is defined
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

imageBase=${sciContImageBase}.beam${BEAM}

shapeDefinition="# Leave shape definition to Cimager to determine from the data"
if [ $pixsizeCont -gt 0 ]; then
    shapeDefinition="Cimager.Images.shape                            = [${pixsizeCont}, ${pixsizeCont}]"
fi

cellsizeDefinition="# Leave cellsize definition to Cimager to determine from the data"
if [ $cellsizeCont -gt 0 ]; then
    cellsizeDefinition="Cimager.Images.cellsize                         = [${cellsizeCont}arcsec, ${cellsizeCont}arcsec]"
fi

directionDefinition="# Leave direction definition to Cimager to determine from the data"
if [ "${directionSci}" != "" ]; then
    directionDefinition="Cimager.Images.image.${imageBase}.direction    = ${directionSci}"
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
# The following are needed for MFS clean
# This one defines the number of Taylor terms
Cimager.Images.image.${imageBase}.nterms       = ${ntermsSci}
# This one assigns one worker for each of the Taylor terms
Cimager.nworkergroups                           = ${nworkergroupsSci}
# This tells the gridder to weight the visibilities appropriately
Cimager.visweights                              = MFS
# This is the reference frequency - it should lie in your frequency range (ideally in the middle)
Cimager.visweights.MFS.reffreq                  = ${freqContSci}
#
# This defines the parameters for the gridding.
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2600
Cimager.gridder                                 = WProject
Cimager.gridder.WProject.wmax                   = 2600
Cimager.gridder.WProject.nwplanes               = 99
Cimager.gridder.WProject.oversample             = 4
Cimager.gridder.WProject.maxsupport             = 512
Cimager.gridder.WProject.variablesupport        = true
Cimager.gridder.WProject.offsetsupport          = true
#
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 500
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = [0, 3, 10]
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.tolerance                  = 0.01
Cimager.solver.Clean.weightcutoff               = zero
Cimager.solver.Clean.weightcutoff.clean         = false
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 50
Cimager.threshold.minorcycle                    = [30%, 0.9mJy]
Cimager.threshold.majorcycle                    = 1mJy
Cimager.ncycles                                 = 2
Cimager.Images.writeAtMajorCycle                = false
#
Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Cimager.preconditioner.Wiener.robustness        = 0.5
#
Cimager.restore                                 = true
Cimager.restore.beam                            = ${restoringBeamCont}
"
