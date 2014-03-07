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

Channel averaging
-----------------

The first step in imaging is to average the visibilities to 304 1MHz channels. This is done with the **mssplit** command (read :doc:`../calim/mssplit` for further information) - here is a typical parset::

	# Input measurement set
	# Default: <no default>
	vis         = sciencefield_SKADS_5s_2014-02-24-1911.ms
	
	# Output measurement set
	# Default: <no default>
	outputvis   = coarse_sciencefield.ms
	
	# The channel range to split out into its own measurement set
	# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
	# is inclusive of both the start and end, indexing is one-based. 
	# Default: <no default>
	channel     = 1-16416
	
	# Defines the number of channel to average to form the one output channel
	# Default: 1
	width       = 54


Save this parset into a file, say **mssplit.in**. To run this, we need to create a qsub file, say, **mssplit.qsub**::

        #!/bin/bash -l
	#PBS -l walltime=01:00:00
	#PBS -l mppwidth=1
	#PBS -l mppnppn=1
	#PBS -N mssplit
	#PBS -j oe
	
	cd $PBS_O_WORKDIR

	aprun -n 1 -N 1 mssplit -c mssplit.in > mssplit_${PBS_JOBID}.log

This runs as a serial job, using only a single processor. Run this in the usual fashion via::

  qsub mssplit.qsub
	
This step isn't necessary for the calibration observations. **ADD MORE DETAIL HERE - WHY NOT NECESSARY?**

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

Save this parset into a file, say **calibrator-BEAM0.in**. To run this, we need to create a qsub file, say, **calibrator-BEAM0.qsub**::

        #!/bin/bash -l
	#PBS -l walltime=01:00:00
	#PBS -l mppwidth=1
	#PBS -l mppnppn=1
	#PBS -N ccal0
	#PBS -j oe
	
	cd $PBS_O_WORKDIR

	aprun -n 1 -N 1 ccalibrator -c calibrator-BEAM0.in > calibrator-BEAM0_${PBS_JOBID}.log

Again, this runs as a serial job, and can be submitted in the usual fashion via::

  qsub calibrator-BEAM0.qsub

Gains Parameters
................

The gains parameters, one for each polarisation, antenna and beam, are written to a file caldata-BEAM0.dat. This has entries that look like the following::

	gain.g11.0.0 = [0.27819,0.680617]
	gain.g11.0.1 = [1.10205,0.0753065]
	gain.g11.0.2 = [0.554685,-0.540324]
	gain.g11.0.3 = [0.670682,0.959503]
	gain.g11.0.4 = [1.14376,-0.171543]
	gain.g11.0.5 = [1.14408,-0.0398638]
	gain.g11.0.6 = [-0.737403,0.560088]
	gain.g11.0.7 = [1.02336,-0.255945]
	gain.g11.0.8 = [-0.818433,-0.248333]
	gain.g11.1.0 = [-0.631561,-0.409769]
	gain.g11.1.1 = [1.29398,0.052276]
	gain.g11.1.2 = [0.547655,0.834172]
	gain.g11.1.3 = [0.271391,-0.790379]
	gain.g11.1.4 = [0.380629,0.674252]
	gain.g11.1.5 = [0.629311,0.410911]
	gain.g11.1.6 = [-0.0787903,1.00496]
	gain.g11.1.7 = [0.41577,-0.881319]
	gain.g11.1.8 = [0.569721,-0.565723]

This file is just a parset, with each line being a parameter specification. Its format is described in more detail in :doc:`../calim/calibration_solutions`, but the format is basically **gain.pol.antenna.beam**. The only relevant entries from this calibration job are those for beam 0 - that is, those ending in *.0*.
 
Gains for each beam
...................

The above finds the correct gains for beam 0. To solve them for all other beams, we need to do the same for beams 1-8. To do this for beam 1, we copy the parset to **calibrator-BEAM1.in** and change "BEAM0" in the dataset name to "BEAM1". Similarly, copy the qsub file to **calibrator-BEAM1.qsub** and replace "BEAM0" in the filenames with "BEAM1", then submit.

All other parameters (for now) can remain the same. The direction, importantly, is the same as this is the phase centre for the observation, which has been defined such that it is in the centre of the beam of interest for each measurement set.

This is something that can easily be scripted. Here is one possibly solution using a bash script::

	#!/bin/bash -l
	
	NUM=0
	while [ $NUM -lt 9 ]; do
	    parset="calibrator-BEAM${NUM}.in"
	    cat > $parset << EOF
	Ccalibrator.dataset                               = calibrator_J1934m638_5s_2014-02-24-1911_BEAM${NUM}.ms
	< other parset contents follow >
	EOF
	
	    qsubfile="calibrator-BEAM${NUM}.qsub"
	    cat > $qsubfile << EOF
	< qsub contents as above>
	aprun -n 1 -N 1 ccalibrator -c calibrator-BEAM${NUM}.in > calibrator-BEAM${NUM}_\${PBS_JOBID}.log
	EOF
	
	    qsub $qsubfile
	
	    NUM=`expr $NUM + 1`
	done

The next bit still feels a little like hacking, as this isn't quite how *ccalibrator* was designed. Once you have all the *caldata-BEAM?.dat* files, you can extract the relevant beam from each file. To get the calibration solution for beam 0, you could do the following::

  grep "\.0 =" caldata-beam0.dat >> caldata-combined.dat

and similarly for the other beams. This will produce a combined set of calibration solutions that can be applied to the science field.

**DO WE NEED TO DO THE COMBINATION IF WE ARE IMAGING BY BEAM ANYWAY?**

Imaging
-------

To do the imaging we select individual beams and image them independently. This is to replicate what is necessary for actual BETA data due to the lack of ...


The imaging is done similarly to that in the introductory tutorial, with two additions. One, we will select an individual beam from the measurement set, and two, we will add some cleaning. Here is an example parset::

	Cimager.dataset                                 = coarse_sciencefield.ms
	Cimager.Feed                                    = 0
	#
	Cimager.nworkergroups                           = 3
	# Each worker will read a single channel selection
	Cimager.Channels                                = [1, %w]
	#
	Cimager.Images.Names                            = [image.i.clean.sciencefield.SKADS.BEAM0]
	Cimager.Images.shape                            = [2048,2048]
	Cimager.Images.cellsize                         = [10arcsec,10arcsec]
	Cimager.Images.image.i.clean.sciencefield.SKADS.BEAM0.frequency          = [0.9e9,0.9e9]
	Cimager.Images.image.i.clean.sciencefield.SKADS.BEAM0.nchan              = 1
	Cimager.Images.image.i.clean.sciencefield.SKADS.BEAM0.direction          = [12h30m00.00, -45.00.00.00, J2000]
	Cimager.Images.image.i.clean.sciencefield.SKADS.BEAM0.nterms             = 2
	#
	Cimager.visweights                              = MFS
	Cimager.visweights.MFS.reffreq                  = 0.9e9
	#
	Cimager.gridder.snapshotimaging                 = true
	Cimager.gridder.snapshotimaging.wtolerance      = 800
	Cimager.gridder                                 = AWProject
	Cimager.gridder.AWProject.wmax                  = 800
	Cimager.gridder.AWProject.nwplanes              = 99
	Cimager.gridder.AWProject.oversample            = 4
	Cimager.gridder.AWProject.diameter              = 12m
	Cimager.gridder.AWProject.blockage              = 2m
	Cimager.gridder.AWProject.maxfeeds              = 9
	Cimager.gridder.AWProject.maxsupport            = 512
	Cimager.gridder.AWProject.variablesupport       = true
	Cimager.gridder.AWProject.offsetsupport         = true
	Cimager.gridder.AWProject.frequencydependent    = true
	#
	Cimager.solver                                  = Clean
	Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
	Cimager.solver.Clean.niter                      = 5000
	Cimager.solver.Clean.gain                       = 0.5
	Cimager.solver.Clean.scales                     = [0, 3, 10, 30]
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
	Cimager.calibaccess.parset                      = caldata-combined.dat
	Cimager.calibrate.scalenoise                    = true
	Cimager.calibrate.allowflag                     = true
	
Before running this, let's look at a few key features of this parset. First is this::

	Cimager.Feed                                    = 0

This does the selection-by-beam, where we only use data for *feed=0* in the measurement set. 

We are doing multi-frequency synthesis for this image. This is controlled by the following parameters::

	Cimager.visweights                              = MFS
	Cimager.visweights.MFS.reffreq                  = 0.9e9

This will result in the creation of "Taylor-term images". These represent the Taylor terms that represent the frequency dependence of each spatial pixel. The different terms relate to the spectral index (alpha) and spectral curvature (beta) of the spectrum, which can be defined through a second-order polynomial in log-space, shown in the first equation below. The second equation shows the result of a Taylor expansion about the reference frequency

.. image:: figures/MFS_formulae.png
   :width: 99%

The Taylor term images then contain the coefficients of this expansion, so that the image with suffix *.taylor.0* contains I_0, *.taylor.1* contains I_0*alpha, and *.taylor.2* contains I_0*(0.5*alpha*(alpha-1)+beta).

The cleaning is controlled by these parameters::

	Cimager.solver                                  = Clean
	Cimager.solver.Clean.algorithm                  = BasisfunctionMFS

and those following. The algorithm *BasisfunctionMFS* is necessary to do the multi-scale multi-frequency synthesis. Read :doc:`../calim/solver` for information on all the clean options. Note that the above parset has::

	Cimager.Images.writeAtMajorCycle                = false

Setting this to true can be useful if you want to look at the intermediate major cycles of the cleaning, but it does produce a lot more images. To save clutter we'll keep it at *false* for now.

To run the imaging, we need a qsub file - call it **clean-BEAM0.qsub**::

	#!/bin/bash -l
	#PBS -l walltime=02:00:00
	#PBS -l mppwidth=913
	#PBS -l mppnppn=16
	#PBS -N clean0
	#PBS -j oe
	
	cd $PBS_O_WORKDIR
	
	aprun -n 913 -N 16 cimager -c clean-BEAM0.in > clean-BEAM0_${PBS_JOBID}.log

Note that the number of processes has increased compared to the intro tutorial. That's because we are doing MFS imaging, and we have requested::

	Cimager.nworkergroups                           = 3

in the parset. This assigns each Taylor term to a separate processor, to spread the work and help speed things up. You can submit this in the usual way, but if you have run the mssplit job, this may still be going, and it needs that to finish first. You can still submit the imaging job, but make it depend on the successful completion of the mssplit job. If the ID of the mssplit job is 1234.rtc, then you can submit the imaging job via::

  qsub -Wdepend=afterok:1234.rtc clean-BEAM0.qsub

Mosaicing
---------

We repeat the imaging for each beam, imaging only a single beam each time. Once this is done, we need to mosaic the images together to form the final full-field image. This is done with the **linmos** program, information on which can be found at :doc:`../calim/linmos`.
