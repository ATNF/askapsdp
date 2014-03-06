Basic Continuum Imaging Tutorial
=================================

.. note:: This tutorial is in the process of being written, but you probably already worked that out.

This tutorial demonstrates basic continuum imaging using ASKAPsoft, with a simulated observation that aims to replicate a "typical" observation with BETA.

Observation summary
-------------------
The data to be reduced consists two distinct parts.

The first has 9 short (6 minute) simulated observations of 1934-638, one for each of the 9 beams. Each observation has its own measurement set. These used the BETA array, covering 304 MHz at full spectral resolution (16416 channels), between 746 and 1050 MHz. Thermal noise (Tsys=50K), plus a set of random gains, have been added to the visibilities. Summary of the specifications:

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


The second is a long (12 hour) simulated observation of a "science field". The sky model used in the field is our standard "SKADS" field, which uses continuum sources from the SKADS S3-SEX simulation (`Wilman et al (2008), MNRAS 388, 1385`_). Each component has its own spectral index, which should be recoverable from the imaging results. The observation had the same spectral backend as the calibration observations, as well as thermal noise and gains corruption (using the same random gains as the calibration observations). Summary of the specifications:

* Target: SKADS field
* Direction: RA=12:30:00.00, Dec=-45:00:00.0 (J2000)
* Observation length: 12 hours
* Integration time: 5 seconds
* Number of beams: 9 
* Polarisations: XX & YY
* Number of spectral channels: 16416
* Channel width: 18.518518 kHz
* Tsys: 50K
* Gains corruption: Yes

 .. _Wilman et al (2008), MNRAS 388, 1385: http://adsabs.harvard.edu/abs/2008MNRAS.388.1335W

Prerequisites
-------------
You should read the :doc:`../platform/processing` documentation and in particular have
setup your environment per the section entitled "Setting up your account".

You should also have read the :doc:`intro` and be comfortable with submitting jobs
and monitoring their status.

Setting up a working directory
------------------------------
Your working directory will not be within your home directory, instead it will reside
on the fast Lustre filesystem::

    cd /scratch/askap/$USER
    mkdir continuumtutorial
    cd continuumtutorial

Retrieving the tutorial dataset
-------------------------------
I have not yet put the measurement sets for this tutorial on the commissioning archive.
Here's how you can get the working copies I've been using for testing. Most likely these will be put on the archive as the standard sets for the tutorial, but they aren't there yet::

  cp -r /scratch/askap/whi550/Simulations/DC2/FullBandwidth/MS/calibrator_J1934m638_5s_2014-02-24-1911_BEAM?.ms .
  cp -r /scratch/askap/whi550/Simulations/DC2/FullBandwidth/MS/sciencefield_SKADS_5s_2014-02-24-1911.ms
  
Note that both of these are at the full spectral resolution, and so each calibrator MS is 2.6GB in size, while the science field MS is 291GB.

Calibration
-----------

The first step is to use the **ccalibrator** program to solve the gains calibration. This will be done using the calibrator observations, and will be applied to the science observation. 

Here is a basic parameter set for use with ccalibrator. It has the same sort of structure as the imaging one you would have seen in the intro tutorial, with a few calibration-specific parameters. As ususal, refer to the documentation pages on calibration, gridding and so forth for more details.::

	Ccalibrator.dataset                               = calibrator_J1934m638_5s_2014-02-24-1911_BEAM0.ms
	Ccalibrator.nAnt                                  = 6
	Ccalibrator.nBeam                                 = 9
	Ccalibrator.solve                                 = gains
	Ccalibrator.interval                              = 360
	#						  
	Ccalibrator.calibaccess                           = parset
	Ccalibrator.calibaccess.parset                    = caldata-BEAM0.dat
	#						  
	Ccalibrator.sources.names                         = [field1]
	Ccalibrator.sources.field1.direction	          = [19h39m25.036, -63.42.45.63, J2000]
	Ccalibrator.sources.field1.components             = src
	Ccalibrator.sources.src.calibrator                = "1934-638"
	#						  
	Ccalibrator.gridder.snapshotimaging               = true
	Ccalibrator.gridder.snapshotimaging.wtolerance    = 800
	Ccalibrator.gridder                               = AWProject
	Ccalibrator.gridder.AWProject.wmax                = 800
	Ccalibrator.gridder.AWProject.nwplanes            = 129
	Ccalibrator.gridder.AWProject.oversample          = 4
	Ccalibrator.gridder.AWProject.diameter            = 12m
	Ccalibrator.gridder.AWProject.blockage            = 2m
	Ccalibrator.gridder.AWProject.maxfeeds            = 9
	Ccalibrator.gridder.AWProject.maxsupport          = 512
	Ccalibrator.gridder.AWProject.variablesupport     = true
	Ccalibrator.gridder.AWProject.offsetsupport       = true
	Ccalibrator.gridder.AWProject.frequencydependent  = true
	#						  
	Ccalibrator.ncycles                               = 5

This parset will solve for the gains for the first calibrator observation. We only care about the BEAM 0 from this observation (which is the beam pointing at 1934), but the task actually tries to solve for all beams.

The calibration is done assuming a model of 1934-638 (the *Ccalibrator.sources.src.calibrator="1934-638"* entry) - this is a special unresolved component that accounts for 1934's spectral variation. It puts the component at the position indicated, which happens to be the direction of the observation.

Imaging
-------
