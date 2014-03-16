Continuum Imaging Tutorial, Part 2
==================================

.. note:: This tutorial is in the process of being written, so may be lacking in detail in some areas. And the data isn't in place yet anyway.

This tutorial demonstrates some more basic continuum imaging using ASKAPsoft, simulating an observation of the standard BETA test field.

Observation summary
-------------------
The data to be reduced consists of two distinct parts.

The first, as for the first continuum imaging tutorial, has 9 short (6 minute) simulated observations of 1934-638, one for each of the 9 beams. Each observation has its own measurement set. These used the BETA array, covering 304 MHz at full spectral resolution (16416 channels), between 746 and 1050 MHz. Thermal noise (Tsys=50K), plus a single set of random gains, have been added to the visibilities. Summary of the specifications:

* Target: 1934-638
* Direction: RA=19:39:25.036, Dec=-63:42:45.63 (J2000)
* Observation length: 6 minutes per beam
* Integration time: 5 seconds
* Number of beams: 9 per observation
* Polarisations: XX & YY
* Number of spectral channels: 16416
* Channel width: 18.518518 kHz
* Tsys: 50K
* Gains corruption: Yes


The second is a long (12 hour) simulated observation of the field that has been used during commissioning for the multi-beam BETA imaging tests - J1600-790. The sky model used for the simulation is developed from the SUMSS catalogue: we take the components from the catalogue and put them on an image. As there is no spectral-index information provided in SUMSS, we use the same single-channel image for all frequency channels, which essentially means everything in the field has a flat spectrum.

The observation uses the same spectral backend as the calibration observations, as well as thermal noise and gains corruption (using the same random gains as the calibration observations). Summary of the specifications:

* Target: J1600-790
* Direction: RA=15:56:58.870, Dec=-79:14:04.28(J2000)
* Observation length: 12 hours
* Integration time: 5 seconds
* Number of beams: 9 
* Polarisations: XX & YY
* Number of spectral channels: 16416
* Channel width: 18.518518 kHz
* Tsys: 50K
* Gains corruption: Yes

Prerequisites
-------------
You should read the :doc:`../platform/processing` documentation and in particular have
setup your environment per the section entitled "Setting up your account". 

You should also have read the :doc:`intro` and :doc:`basiccontinuum`, be comfortable with submitting jobs and monitoring their status, and familiar with the calibration & imaging steps.

Setting up a working directory
------------------------------
Your working directory will not be within your home directory, instead it will reside
on the fast Lustre filesystem::

    cd /scratch/askap/$USER
    mkdir continuumtutorial_v2
    cd continuumtutorial_v2

Retrieving the tutorial dataset
-------------------------------

As before, there are 11 measurement sets associated with this tutorial. There are nine for the calibration observations (one per beam), named calibrator_J1934m638_forSKADS_BEAM0.ms through calibrator_J1934m638_forBETAtestfield_BEAM8.ms (these are 2.6GB each). The science field has one at full spectral resolution, named sciencefield_BETAtestfield.ms (291GB), and an averaged version of this named sciencefield_BETAtestfield_coarseChan.ms (5.5GB). You can start with the full-resolution version, and run the channel averaging yourself (see below), or you can start with the latter, and skip the channel averaging. 

The measurement sets reside on the "Commissioning Archive" and can be retrieved using the scp command. As the measurement sets may need to be fetched from tape, they should be staged first::

    ssh cortex.ivec.org /opt/SUNWsamfs/bin/stage -r /pbstore/groupfs/askap/tutorials/basiccontinuum/sciencefield_SKADS.ms
    scp -r cortex.ivec.org:/pbstore/groupfs/askap/tutorials/basiccontinuum/sciencefield_SKADS.ms .

and similarly for the other measurement sets.

You may notice the **scp** may stall. This is likely due to the fact the data has not been fetched (staged) from tape to disk. This is quite normal, and the length of the stall depends upon the load on the system (e.g. other users).

Calibration
-----------

These observations are calibrated in the same fashion as for the first continuum imaging tutorial :doc:`basiccontinuum`. We therefore won't repeat the explanation here, but this step is necessary.

Channel averaging
-----------------



Imaging
-------

We image this data set in the same way as for the SKADS simulation described in :doc:`basiccontinuum`. However, because of the nature of the simulation (no frequency dependence in the sky model) we can do it in a slightly different way. We can choose not to do the multi-frequency synthesis, and just make a single-channel image at the central frequency. Here is an example parset for Beam 0::

	Cimager.dataset                                 = sciencefield_BETAtestfield_coarseChan.ms
	Cimager.Feed                                    = 0
	#
	# Each worker will read a single channel selection
	Cimager.Channels                                = [1, %w]
	#
	Cimager.Images.Names                            = [image.i.clean.BETAtestfield.BEAM0]
	Cimager.Images.shape                            = [3072,3072]
	Cimager.Images.cellsize                         = [10arcsec,10arcsec]
	Cimager.Images.image.i.clean.BETAtestfield.BEAM0.frequency          = [0.9e9,0.9e9]
	Cimager.Images.image.i.clean.BETAtestfield.BEAM0.nchan              = 1
	Cimager.Images.image.i.clean.BETAtestfield.BEAM0.direction          = [15h56m58.870,-79.14.04.28, J2000]
	#
	Cimager.gridder.snapshotimaging                 = true
	Cimager.gridder.snapshotimaging.wtolerance      = 2800
	Cimager.gridder                                 = AWProject
	Cimager.gridder.AWProject.wmax                  = 2800
	Cimager.gridder.AWProject.nwplanes              = 99
	Cimager.gridder.AWProject.oversample            = 4
	Cimager.gridder.AWProject.diameter              = 12m
	Cimager.gridder.AWProject.blockage              = 2m
	Cimager.gridder.AWProject.maxfeeds              = 9
	Cimager.gridder.AWProject.maxsupport            = 2048
	Cimager.gridder.AWProject.variablesupport       = true
	Cimager.gridder.AWProject.offsetsupport         = true
	Cimager.gridder.AWProject.frequencydependent    = true
	#
	Cimager.solver                                  = Clean
	Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
	Cimager.solver.Clean.niter                      = 5000
	Cimager.solver.Clean.gain                       = 0.5
	Cimager.solver.Clean.scales                     = [0, 3, 10]
	Cimager.solver.Clean.verbose                    = False
	Cimager.solver.Clean.tolerance                  = 0.01
	Cimager.solver.Clean.weightcutoff               = zero
	Cimager.solver.Clean.weightcutoff.clean         = false
	Cimager.solver.Clean.psfwidth                   = 512
	Cimager.solver.Clean.logevery                   = 100
	Cimager.threshold.minorcycle                    = [30%, 0.9mJy]
	Cimager.threshold.majorcycle                    = 1mJy
	Cimager.ncycles                                 = 5
	Cimager.Images.writeAtMajorCycle                = false
	#
	Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
	Cimager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
	Cimager.preconditioner.Wiener.robustness        = 0.0
	Cimager.preconditioner.Wiener.taper             = 64
	#
	Cimager.restore                                 = true
	Cimager.restore.beam                            = fit
	#
	# Apply calibration
	Cimager.calibrate                               = true
	Cimager.calibaccess                             = parset
	Cimager.calibaccess.parset                      = caldata-BEAM0.dat
	Cimager.calibrate.scalenoise                    = true
	Cimager.calibrate.allowflag                     = true

