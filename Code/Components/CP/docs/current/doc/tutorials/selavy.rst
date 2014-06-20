Selavy source-finding tutorial
==============================

This tutorial demonstrates the use of the Selavy sourcefinder to find
sources in an image, and fit components to them. This will take the
image produced from the :doc:`basiccontinuum` tutorial and demonstrate
different approaches to building up a source catalogue.

The various options of Selavy are described in detail in the
:doc:`../analysis/index` section, and this tutorial will not attempt
to demonstrate all possible modes. We will focus primarily on the
needs of continuum imaging.

Prerequisites
-------------

This tutorial uses the outputs from the imaging tutorial, specifically
the restored Taylor term images and, optionally, the weights
images. However, the scripts can be used with essentially any image:
Selavy is able to read both CASA and FITS format. To find the spectral
index & curvature of sources, you will need Taylor term images - these
are found automatically if the input image is named in the same way as
the output from cimager, but you can directly specify them (see
:doc:`../analysis/postprocessing` for details).


A simple search
---------------

The first demonstration will be a simple search to a given flux
threshold, in this case 100mJy. Sources that lie above this threshold
are "grown" to a secondary threshold of 50mJy, and then have Gaussian
components fitted to them.

Pixels that lie above the primary threshold are identified and grouped
together into 'islands'. These are then grown out to include all
neighbouring pixels above the secondary threshold - pixels with fluxes
between these two thresholds are only included if they are attached to
pixels above the primary threshold.

When fitting is requested, Gaussian components are fitted to the
islands and their parameters written to a separate catalogue. It is
typically this catalogue that one would use in analysing continuum
observations, or, for instance to construct a sky model.

Here is the parset used for this search:

.. code-block:: bash

    # The image to be searched
    Selavy.image = image.i.clean.sciencefield.linmos.taylor.0.restored
    #
    # We will search just a subsection of the image
    Selavy.flagSubsection = true
    Selavy.subsection = [251:1800,251:1800,*,*]
    #
    # This is how we divide it up for distributed processing, with the
    #  number of subdivisions in each direction, and the size of the
    #  overlap region in pixels
    Selavy.nsubx = 6
    Selavy.nsuby = 3
    Selavy.overlapx = 50
    Selavy.overlapy = 50
    #
    # The search thresholds, in the flux units of the image
    Selavy.threshold = 0.1
    Selavy.flagGrowth = true
    Selavy.growthThreshold = 0.05
    #
    # Parameters to switch on and control the Gaussian fitting
    Selavy.Fitter.doFit = true
    Selavy.Fitter.fitTypes = [full]
    Selavy.Fitter.maxNumGauss = 2
    #
    # Size criteria for the final list of detected islands
    Selavy.minPix = 3
    Selavy.minVoxels = 3
    Selavy.minChannels = 1
    #
    # How the islands are sorted in the final catalogue - by
    #  integrated flux in this case
    Selavy.sortingParam = iflux
		

If one saves this to a parset file **selavy.in**, say, then it can be
run using the following sbatch file:

.. code-block:: bash

    #!/usr/bin/env bash
    #SBATCH --time=01:00:00
    #SBATCH --ntasks=19
    #SBATCH --ntasks-per-node=19
    #SBATCH --job-name=selavy
    
    log=selavy-${SLURM_JOB_ID}.log
    
    aprun -N 19 -n 19 selavy -c selavy.in > $log

This example demonstrates several elements of Selavy. First, it is run
in distributed mode, where the image is split up so that each worker
process only processes a small fraction of the image. The way it is
split up is governed by the *nsubx* and *nsuby* parameters, being the
number of subimages in the x- and y-directions, and the overlap
between subimages (in pixels) is controlled by the *overlapx* and
*overlapy* parameters. The overlap ensures we don't miss structure at
the boundaries. 

The fitting is switched on by setting *Fitter.doFit = true* (it is off
by default). There are many options possible in the fitting, which are
detailed in :doc:`../analysis/postprocessing`. The setup in this
example limits the fitting to either one or two Gaussian components -
one is fit first and is accepted if the fit works and passes
chi-squared & RMS tests, else a two component fit is tried. If both
fail, the initial estimates are returned.

The key outputs are as follows:

+--------------------------------+-----------------------+--------------------------------------+
|Filename                        | Parameters            | Description                          |
+================================+=======================+======================================+
|selavy-results.txt              |resultsFile            |The catalogue of detected islands and |
|                                |                       |their parameters.                     |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-results.xml              |votFile                |A VOTable version of the islands      |
|                                |                       |catalogue.                            |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-results.ann              |karmaFile / flagKarma  |A karma annotation file showing the   |
|                                |                       |locations of the islands.             |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-results.reg              |ds9File / flagDS9      |A DS9 region file showing the island  |
|                                |                       |locations                             |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-results.crf              |casaFile / flagCASA    |A CASA region file showing the island |
|                                |                       |locations                             |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-fitResults.txt           |fitResultsFile         |The catalogue of fitted components.   |
|                                |                       |                                      |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-fitResults.xml           |                       |A VOTable version of the fitted       |
|                                |                       |components catalogue.                 |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-fitResults.{ann,reg,crf} |fitAnnotationFile      |Annotation/region files showing the   |
|                                |                       |location of the fitted components.    |
+--------------------------------+-----------------------+--------------------------------------+
|selavy-SubimageLocations.ann    |subimageAnnotationFile |Karma annotation file showing the     |
|                                |                       |borders of the subimages.             |
+--------------------------------+-----------------------+--------------------------------------+
|                                |                       |                                      |
+--------------------------------+-----------------------+--------------------------------------+

There are two catalogues written: *selavy-results.txt* is the
Duchamp-style catalogue showing the detected islands and their
parameters; and *selavy-fitResults.txt* is the catalogue of fitted
components. The ID of the fitted components has a number and a
letter - the number refers to the ID of the island in
*selavy-results.txt*, while the letter gives the order of the
component (ordered by flux) within that island.

The *.txt* files are simple formatted ASCII tables. They have
XML/VOTable equivalents  written as well - these are standard VOTable
format that can be used in applications such as TOPCAT and Aladin. 

There are annotation and region files produced that allow the detected
sources to be displayed in either kvis (the *.ann* files), DS9 (the
*.reg* files) or casaviewer (either *.reg* or *.crf* are
possible). These will show the outline of the detected island, or
ellipses indicating the fitted components. There is also a karma
annotation file showing the locations of the worker subimages. 



Simple Signal-to-noise thresholding
-----------------------------------

A simple change to the above parset can make your search be in
signal-to-noise terms. Change 

.. code-block:: bash

    # The search thresholds, in the flux units of the image
    Selavy.threshold = 0.1
    Selavy.flagGrowth = true
    Selavy.growthThreshold = 0.05

to

.. code-block:: bash

    # The search thresholds, in units of sigma above the mean
    Selavy.snrCut = 10
    Selavy.flagGrowth = true
    Selavy.growthCut = 4

and you will do a search for sources above 10-sigma, that are then
grown to the 4-sigma level.

The default approach uses the median and median absolute deviation
from the median (MADFM), although the mean and standard deviation can
be used by setting *Selavy.flagRobustStats=false*. The robust
statistics take a little longer, due to the partial sorting that is
involved, but they are probably more reliable, particularly when there
is a lot of signal present.

This approach uses the Duchamp method of applying a single threshold
to the entire image. When run in distributed mode, as this will be,
each worker finds the mean (or median), sends it to the master process
which averages them to find the global mean estimator. This is then
distributed to the workers who then find their local standard
deviation or MADFM. These are again averaged by the master to provide
a global estimator of the standard deviation, and hence the
threshold. A more complete description of this process can be found in
`Whiting & Humphreys (2012), PASA 29, 371`_.

 .. _Whiting & Humphreys (2012), PASA 29, 371: http://www.publish.csiro.au/paper/AS12028.htm 

This will take slightly longer than the first example, due to the
additional statistics calculation & communication, but will still be
quick - my example took about 20 seconds to run, finding 560 sources. 


Variable signal-to-noise thresholding
-------------------------------------

Instead of only using a single global threshold, Selavy allows the use
of signal-to-noise thresholds that depend only on the noise local to a
given pixel. This allows the actual flux threshold to vary according
to the image noise, rising where the sensitivity or background due to
sidelobes is high, and falling where the sensitivity is very good.

To make use of this, keep the signal-to-noise threshold parameters as
shown above, and add the following:

.. code-block:: bash

    # Turn on the variable threshold option
    Selavy.VariableThreshold = true
    # Pixel width of box in which to calculate statistics.
    Selavy.VariableThreshold.boxSize = 50
    # Name of image of detection threshold to be created 
    Selavy.VariableThreshold.ThresholdImageName=detThresh.img
    # Name of image of noise to be created 
    Selavy.VariableThreshold.NoiseImageName=noiseMap.img
    # Name of image of mean pixel level to be created 
    Selavy.VariableThreshold.AverageImageName=meanMap.img
    # Name of image of signal-to-noise ratio to be created 
    Selavy.VariableThreshold.SNRimageName=snrMap.img

The key parameters are *VariableThreshold* and
*VariableThreshold.boxSize*, which turn on the feature and set the
size of the box in which the statistics and threshold are
calculated. A square box of this width is slid over the image, and
each pixel has a threshold calculated from the noise statistics of the
pixels within the box centred on it.

Images showing the various statistics can be written - only those
given in the parset will be written. All images will be the same size
as the input image (regardless of what search subsection you have
used). 

This method also allows you to more confidently drop the detection
threshold from those levels used with the global threshold case above,
as you won't be swamped by spurious signal in high-noise areas.

Again, this works with the distributed processing. The overlaps
between subimages will be at least twice the box width, as pixels
within a box width of the edge of a worker's image will not be
processed (note this if you have things near the edge of your image
that you care about).

The execution time for this mode is a bit longer, as the sliding-box
calculations require quite a bit more processing (particularly if the
robust statistics are used). Even so, my tests completed in about 1.5
minutes, finding about 800 sources.


Alternative modes
-----------------

This section will detail a few alternative ways of running Selavy

Finding initial component estimates
...................................

A key part of the Gaussian fitting is determining an initial estimate
of the parameters. Selavy's original approach (and currently the
default) was to apply a number of "sub-thresholds" in between the
detection threshold and the peak, and track the number of separate
maxima within the island. There are a range of parameters, detailed on
:doc:`../analysis/postprocessing`, that control the details of this
algorithm. 

While this approach works well, I've recently incorporated the
algorithm used in Aegean, which uses a curvature map to locate local
maxima. See `Hancock et al. (2012) MNRAS 422, 1812`_ for details on
the algorithm. I'm currently doing a detailed comparison of the two
approaches, but this mode is available in CP-0.3.

 .. _Hancock et al. (2012) MNRAS 422, 1812: http://adsabs.harvard.edu/abs/2012MNRAS.422.1812H

To make use of this option, add the following to your parset:

.. code-block:: bash

    # Parameters to switch on curvature map analysis and output the
    #  curvature image
    Selavy.Fitter.useCurvature = true
    Selavy.Fitter.curvatureMap = curvature.img

Forcing a PSF fit
.................

It may be that you want to force the Gaussian fitting to fit only
PSF-sized components, rather than allowing the size of the components
to expand.

To do this, you can choose an alternative mode for the fitting by
putting this in your parset:

.. code-block:: bash

    # This forces the fits to just use PSF-sized components
    Selavy.Fitter.fitTypes = [psf]

This will then just fit to the location and flux of the component,
keeping the size fixed to that of the beam (as specified in the image
header). 


