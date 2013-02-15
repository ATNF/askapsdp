Selavy Documentation
========================

Summary
-------

Selavy is the ASKAP implementation of the source-finding algorithms of Duchamp (publicly available at the `Duchamp website`_). Duchamp is a source-finder designed for three-dimensional datasets (such as HI spectral-line cubes), but is easily applied to two- or one-dimensional data. There is an extensive user's guide available on the Duchamp website that provides detail about the algorithms. Users are also referred to the Duchamp journal paper `Whiting (2012), MNRAS 421, 3242`_ for further details and examples.

Selavy uses the core Duchamp algorithms, and adds new features as dictated by the requirements of ASKAP processing. See `Whiting & Humphreys (2012), PASA 29, 371`_
for descriptions and some benchmarks. The new features include support for distributed processing, Gaussian fitting to two-dimensional images, variable-threshold detection, and additional pre- and post-processing algorithms specified by the Survey Science Teams.

 .. _Duchamp website: http://www.atnf.csiro.au/people/Matthew.Whiting/Duchamp
 .. _Whiting (2012), MNRAS 421, 3242: http://onlinelibrary.wiley.com/doi/10.1111/j.1365-2966.2012.20548.x/full
 .. _Whiting & Humphreys (2012), PASA 29, 371: http://www.publish.csiro.au/paper/AS12028.htm 

Basic Execution and Control Parameters
--------------------------------------

This section summarises the basic execution of Selavy, with control of specific types of processing discussed in later sections. The default approach of Selavy is to follow the Duchamp method of applying a single threshold, either a flux value (specified by the user) or a signal-to-noise level, to the entire image. Pixels above this threshold are grouped together in objects - they may be required to be contiguous, or they can be separated by up to some limit. Detected objects may be "grown" to a secondary (lower) threshold, to include faint pixels without having to search to a faint level. 

The image can be subdivided for distributed processing. To get a single SNR threshold in this case, the noise stats are calculated on a subimage basis and combined to find the average noise across the image. 

The following table lists the basic parameters governing the searching. All parameters listed here should have a **Selavy.** prefix (ie. **Selavy.image**)

General parameters
~~~~~~~~~~~~~~~~~~

+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|*Parameter*         |*Type*              |*Default*           |*Description*                                                               |
+====================+====================+====================+============================================================================+
|image               |string              |none                |The input image - can also use **imageFile** for consistency with Duchamp   |
|                    |                    |                    |parameters                                                                  |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|nsubx               |int                 |1                   |The number of subdivisions in the x-direction when making the subimages.    |
|                    |                    |                    |                                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|nsuby               |int                 |1                   |The number of subdivisions in the y-direction when making the subimages.    |
|                    |                    |                    |                                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|nsuby               |int                 |1                   |The number of subdivisions in the z-direction when making the subimages.    |
|                    |                    |                    |                                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|overlapx            |int                 |0                   |The number of pixels of overlap between neighbouring subimages in the       |
|                    |                    |                    |x-direction                                                                 |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|overlapy            |int                 |0                   |The number of pixels of overlap between neighbouring subimages in the       |
|                    |                    |                    |y-direction                                                                 |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|overlapz            |int                 |0                   |The number of pixels of overlap between neighbouring subimages in the       |
|                    |                    |                    |z-direction                                                                 |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|subimageAnnotationF\|string              |""                  |The filename of a Karma annotation file that is created to show the         |
|ile                 |                    |                    |boundaries of the subimages (see description below). If empty, no such file |
|                    |                    |                    |is created.                                                                 |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|useCASAforFITS      |bool                |true                |Whether to use the casacore functionality for FITS images (if false, the    |
|                    |                    |                    |native Duchamp code is used - see discussion above)                         |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|searchType          |string              |spatial             |How the searches are done: in 2D channel maps ("spatial") or in 1D spectra  |
|                    |                    |                    |("spectral")                                                                |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|snrCut              |float               |4.                  |Threshold value, in multiples of the rms above the mean noise level         |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|threshold           |float               |-9999.9             |Threshold value in units of the FITS cube (default value means snrCut will  |
|                    |                    |                    |be used instead)                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagGrowth          |bool                |false               |Whether to grow detections to a lower threshold                             |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|growthCut           |float               |3.                  |Signal-to-noise ratio to grow detections down to                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|growthThreshold     |float               |0.                  |Threshold value to grow detections down to (takes precedence over           |
|                    |                    |                    |**growthCut**)                                                              |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagNegative        |bool                |false               |Whether to invert the cube and search for negative features                 |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagRobust          |bool                |true                |Whether to use robust statistics when evaluating image noise statistics.    |
|                    |                    |                    |                                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|minPix              |int                 |2                   |Minimum number of pixels allowed in a detection                             |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|minChannels         |int                 |3                   |Minimum number of channels allowed in a detection                           |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagAdjacent        |bool                |true                |When merging sources, whether sources need to be adjacent to be merged      |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|threshSpatial       |float               |3.                  |When flagAdjacent=false, this is the spatial threshold (in pixels) within   |
|                    |                    |                    |which objects are combined                                                  |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|threshVelocity      |float               |7.                  |When flagAdjacent=false, this is the frequency threshold (in channels)      |
|                    |                    |                    |within which objects are combined                                           |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagSubsection      |bool                |false               |Whether to look at just a subsection of the given image                     |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|subsection          |string              |full image          |A subsection string, e.g. **[1001:2000, 50:1572, *, *]** - only these pixels|
|                    |                    |                    |will be examined                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagStatSec         |bool                |false               |Whether to use a subsection of the image to calculate the image statistics  |
|                    |                    |                    |                                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|statsec             |string              |full image          |A subsection string indicating the pixel region to be used to calculate     |
|                    |                    |                    |statistics (mean, rms,...)                                                  |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|-outFile-           |string              |duchamp-Results.txt |The text file holding the results. *Value held fixed - parameter has no     |
|                    |                    |                    |effect.*                                                                    |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagSeparateHeader  |bool                |false               |Whether the "header" containing the summary of input parameters should be   |
|                    |                    |                    |written to a separate file from the table of results. If produced, it will  |
|                    |                    |                    |be called selavy-results.hdr.                                               |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagLog             |bool                |true                |Produce a Duchamp-style log file, recording intermediate detections (see    |
|                    |                    |                    |below). *Note the different default from standard Duchamp.* The workers will|
|                    |                    |                    |produce selavy-Logfile%w.txt, (where %w is the worker number, in the usual  |
|                    |                    |                    |fashion) and the master will produce selavy-Logfile-Master.txt.             |
|                    |                    |                    |                                                                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagKarma           |bool                |true                |Produce a Karma annotation plot (see below). *Note the different default    |
|                    |                    |                    |from standard Duchamp.* Filename will be selavy-results.ann                 |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagVOT             |bool                |true                |Produce a VOTable of the results. Filename will be selavy-results.xml.      |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|drawBorders         |bool                |true                |Whether to draw the object borders in the annotation file. If false, only a |
|                    |                    |                    |circle is drawn with radius proportional to the object's size.              |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|precFlux            |int                 |3                   |Precision for the flux values in the output files                           |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|precVel             |int                 |3                   |Precision for the velocity values in the output files                       |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|precSNR             |int                 |2                   |Precision for the SNR values in the output files                            |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|flagMW              |bool                |false               |Whether to ignore a range of channels that might be affected by say the     |
|                    |                    |                    |Milky Way (eg. in HIPASS cubes)                                             |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|minMW               |int                 |75                  |The minimum channel number to be ignored                                    |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|maxMW               |int                 |112                 |The maximum channel number to be ignored                                    |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|beamArea            |float               |10.                 |The area of the beam in *pixels*. This parameter is only used when the image|
|                    |                    |                    |does not provide beam information. When this is used, a circular beam is    |
|                    |                    |                    |assumed.                                                                    |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|beamFWHM            |float               |-1.                 |The FWHM of the beam in *pixels*. This parameter is only used when the image|
|                    |                    |                    |does not provide beam information. When this is used, a circular beam is    |
|                    |                    |                    |assumed. This value takes precedence over **beamArea** but is ignored if    |
|                    |                    |                    |negative (the default).                                                     |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|verbose             |bool                |false               |Controls the verbosity for the Duchamp-specific code. **verbose=true** means|
|                    |                    |                    |more information about the Duchamp functions                                |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|pixelCentre         |string              |centroid            |How the central pixel value is defined in the output catalogues             |
|                    |                    |                    |(centroid/average/peak).                                                    |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|spectralUnits       |string              |km/s                |The units desired for the spectral axis.                                    |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+
|sortingParam        |string              |vel                 |Which parameter to sort the output list by: x-value, y-value, z-value, ra,  |
|                    |                    |                    |dec, vel, w50, iflux, pflux, snr. A - prepended to the parameter reverses   |
|                    |                    |                    |the order of the sort.                                                      |
+--------------------+--------------------+--------------------+----------------------------------------------------------------------------+


Control switches
~~~~~~~~~~~~~~~~

+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|*Parameter*         |*Type*              |*Default*           |*Description*                                                  |
+====================+====================+====================+===============================================================+
|flagATrous          |bool                |false               |Use the a trous wavelet reconstruction algorithm. See          |
|                    |                    |                    |Preprocessing_ page for details.                               |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|flagSmooth          |bool                |false               |Use spectral or spatial smoothing prior to source-finding. See |
|                    |                    |                    |Preprocessing_ page for details.                               |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|recon2D1D           |bool                |false               |Use the 2D1D wavelet reconstruction algorithm (provided by     |
|                    |                    |                    |WALLABY). See Preprocessing_ page for details.                 |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|optimiseMask        |bool                |false               |Whether to use the mask optimisation algorithm to optimally    |
|                    |                    |                    |increase the size of each object.                              |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|extractSpectra      |bool                |false               |Extract a spectrum (to a CASA image) for each detected source. |
|                    |                    |                    |See Extraction_ page for details.                              |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|extractNoiseSpectra |bool                |false               |Extract a noise spectrum (to a CASA image) for each detected   |
|                    |                    |                    |source. See Extraction_ page for details.                      |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+
|Fitter.doFit        |bool                |false               |Fit Gaussian components to objects detected in a               |
|                    |                    |                    |two-dimensional image                                          |
+--------------------+--------------------+--------------------+---------------------------------------------------------------+

.. _Preprocessing: preprocessing.html
.. _Extraction: extraction.html


Mask optimisation
-----------------

Selavy now has an implementation of WALLABY's mask optimisation algorithm. This allows the mask of a source to be grown out to maximise the integrated flux of the source. Unlike the Duchamp-derived "growing" algorithm (flagGrowth=true), the growing process does not use a flux or signal-to-noise threshold. Rather, it adds elliptical annuli to the source until the total flux of the annulus is negative or a specified maximum number of iterations is reached. It does this for all channels within +-W50 of the central spectral channel.

Algorithm details
~~~~~~~~~~~~~~~~~

The algorithm is implemented in the following way:

1. Source detection is performed using the usual Duchamp/Selavy approaches. Note that the growing option can be used prior to the mask optimisation.
2. First step for a given object is to define the spectral range over which the optimisation is done. This is taken to be W50 either side of the centre of the detection (in turn, defined by the Duchamp parameter pixelCentre). All channels within this range are treated equally.
3. An ellipse is then fitted to the moment-0 map of the object. The moment-0 map uses the detected voxels only, so is dependent on the source-detection done prior to the mask optimisation. 
4. If **clobberPrevious=true**, the source is pared back to the single pixel at the centre, which is used as the seed for the optimisation. Otherwise, we keep the existing mask and build on it.
5. The mask is then grown:

 i. For each spatial pixel in the current mask, all neighbouring pixels not already in the mask that lie within the ellipse are included in a new object. This is done for every channel over the spectral range
 ii. If the flux of this object is positive, each of its pixels are added to the current mask, 
 iii. The size of the ellipse is increased by 1 pixel in the major axis direction, and enough in the minor axis direction to preserve the shape.

6. This continues as long as the flux of the new "object" is positive, and the number of growing iterations is less than **maxIter**.
7. The parameterisation of the object is redone.
8. Once all objects have been done, the Duchamp merging process is re-run to cleanup any pairs of objects that may have intersected.

Note that the flux etc that is calculated for the object after the mask optimisation replaces the value that would have been calculated before it. At this point, if you want to know, say, the integrated flux of an object with and without this mask optimisation, you will need to run the algorithm twice. I am looking at providing this as an *alternative* flux measurement - at that point the code may be put into the Duchamp library proper, rather than just in Selavy.

The result of the mask optimisation does depend somewhat on what is done in the source-detection prior to its use. Using the growing method of Duchamp (**flagGrowth=true** and **growthCut** or **growthThreshold**) allows a good initial estimate of both the spectral range and the ellipse fitted to the moment-0 map. The optimised mask still tends to be larger than that from Duchamp-growing alone (at least from the limited testing done so far).

To aid evaluation of this algorithm, the mask FITS output of Duchamp has been enabled in Selavy, although *only for the serial case*. If running a multiple-node job, the mask will not be written. When written, the mask will be called *selavy-MASK-IMG.fits*, where the IMG refers to the input image. There are also capacity constraints in running this on the Selavy service (to prevent, say, 30GB mask cubes being created repeatedly). Refer to the [[Wiki_sup_wg_2_sourcefinding_service|Selavy page]] for details.

Mask optimisation parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-------------------------------+------------+------------+----------------------------------------------------------+
|*Parameter*                    |*Type*      |*Default*   |*Explanation*                                             |
+===============================+============+============+==========================================================+
|optimiseMask                   |bool        |false       |Whether to use the mask optimisation algorithm            |
+-------------------------------+------------+------------+----------------------------------------------------------+
|optimiseMask.maxIter           |int         |10          |The maximum number of iterations to do. The growing       |
|                               |            |            |process stops if this is reached, or if the flux of the   |
|                               |            |            |annulus is negative                                       |
+-------------------------------+------------+------------+----------------------------------------------------------+
|optimiseMask.clobberPrevious   |bool        |true        |If true, the algorithm starts with only the central pixel |
|                               |            |            |and grows out from there, after fitting an ellipse to the |
|                               |            |            |input detection.  If false, the algorithm starts with the |
|                               |            |            |input mask already determined by the Duchamp algorithms.  |
|                               |            |            |                                                          |
+-------------------------------+------------+------------+----------------------------------------------------------+



Distributed processing
----------------------

The primary innovation in the ASKAPsoft implementation has been to allow distributed processing of images, to test the likely functionality of pipeline processing. The image is split up into subimages according to a user-specified scheme (the user provides the number of subdivisions in the x-, y- and z-directions). Neighbouring subimages can be overlapped by a certain amount (which may be desirable, particularly in the case of variable thresholds - see below). 

Processing is performed under a master-worker framework, where a single master process coordinates the processing, and each worker handles a single subimage. Each of these subimages is searched independently, then the worker sends the list of detected sources to the master process. Once the master has accumulated the full set of detected sources, objects near the overlap regions are merged (if necessary) and have their parameters recalculated. The results are then written out.

Parameters for distributed processing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+--------------------+------------+------------+------------------------------------------------------+
|*Parameter*         |*Type*      |*Default*   |*Description*                                         |
+====================+============+============+======================================================+
|Selavy.nsubx        |int         |1           |The number of subdivisions in the x=direction when    |
|                    |            |            |making the subimages.                                 |
+--------------------+------------+------------+------------------------------------------------------+
|Selavy.nsuby        |int         |1           |The number of subdivisions in the y-direction when    |
|                    |            |            |making the subimages.                                 |
+--------------------+------------+------------+------------------------------------------------------+
|Selavy.nsuby        |int         |1           |The number of subdivisions in the z-direction when    |
|                    |            |            |making the subimages.                                 |
+--------------------+------------+------------+------------------------------------------------------+
|Selavy.overlapx     |int         |0           |The number of pixels of overlap between neighbouring  |
|                    |            |            |subimages in the x-direction                          |
+--------------------+------------+------------+------------------------------------------------------+
|Selavy.overlapy     |int         |0           |The number of pixels of overlap between neighbouring  |
|                    |            |            |subimages in the y-direction                          |
+--------------------+------------+------------+------------------------------------------------------+
|Selavy.overlapz     |int         |0           |The number of pixels of overlap between neighbouring  |
|                    |            |            |subimages in the z-direction                          |
+--------------------+------------+------------+------------------------------------------------------+



Variable thresholds
-------------------

The Duchamp package uses a single detection threshold for the entire image being searched. However, if the sensitivity varies across the field, this will either mean some regions are not searched as deep as they could be and/or some are searched too deeply, resulting in too many spurious detections. The ASKAP implementation deals with this in one of two ways.

The first is to use a weights image, such as that produced by the ASKAPsoft imager (and included in most of the ASKAP simulations), to scale the image according to the sensitivity. In practice, this takes the square root of the normalised weights and divides this into the pixel values. This has the effect of scaling down the low-sensitivity regions of the image, making it less likely that they present many spurious detections. The weights image is specified via **Selavy.weightsimage**. The detection thresholds are provided in the usual fashion. The pixel values are only affected for the detection phase - parameter calculations are *not* affected.

The alternative is to impose a signal-to-noise threshold based on the *local* noise surrounding the pixel in question. This threshold then varies from pixel to pixel based on the change in the local noise. This mode is turned on using the **Selavy.doMedianSearch** parameter, which default to false.

This "local" level is estimated by measuring the median and median absolute deviation from the median of pixels within a box centred on the pixel in question. An array is thus built up containing the signal-to-local-noise values for each pixel in the image, and this array is then searched with a SNR threshold (**Selavy.snrCut**) and, if necessary, grown to a secondary SNR threshold (**Selavy.growthCut**). 

The searching can be done either spatially or spectrally, and this affects how the SNR values are calculated. If spatially (the default), a 2D sliding box filter is used to find the local noise. If spectrally, only a 1D "box" is used. Note that the edges (ie. all pixels within the half box width of the edge) are set to zero, and so detections will not be made there. This probably won't affect the 2D case, as often the edges of the field have poor sensitivity (certainly the ASKAP simulations mostly have a padding region around the edge), but in the 1D case this will mean the loss of the first & last channels. The choice between 2D and 1D is made with the **Selavy.searchType** parameter (which actually comes out of the Duchamp package).

When run on a distributed system as above, this processing is done at the worker level. Note that having an overlap between workers of at least the half box width will give continuous coverage (avoiding the aforementioned edge problems). The amount of processing needed increases quickly with the size of the box, due to the use of medians, particularly for the 2D case. 

A final option for varying the threshold spatially is to use a different threshold for each worker. In this scenario, switched on by setting **thresholdPerWorker = true**, each worker finds its own threshold based on the noise within it. No variation of the threshold *within* a worker is done, so you get discrete jumps in the threshold at worker boundaries. Use of the overlap can mitigate this. This mode was implemented more as an experiment than out of any expectation it would be useful, and limited trials indicate it's probably not much use. For completeness we include the parameter here. 

Parameters for variable threshold
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------------------+------------+------------+-------------------------------------------------+
|*Parameter*                 |*Type*      |*Default*   |*Description*                                    |
+============================+============+============+=================================================+
|Selavy.weightsimage         |string      |""          |The filename of the weights image to be used to  |
|                            |            |            |scale the fluxes prior to searching.  If blank,  |
|                            |            |            |this mode is not used.                           |
+----------------------------+------------+------------+-------------------------------------------------+
|Selavy.doMedianSearch       |bool        |false       |If true, a sliding box function is used to find  |
|                            |            |            |the local median and MADFM (median absolute      |
|                            |            |            |deviation from median), which are used to make a |
|                            |            |            |signal-to-noise map that can be used for         |
|                            |            |            |searching.                                       |
+----------------------------+------------+------------+-------------------------------------------------+
|Selavy.medianBoxWidth       |int         |50          |The half-width of the box used in the SNR map    |
|                            |            |            |calculation. The full width of the box is        |
|                            |            |            |2*medianBoxWidth+1.                              |
+----------------------------+------------+------------+-------------------------------------------------+
|Selavy.searchType           |string      |spatial     |In which sense to do the searching: spatial=2D   |
|                            |            |            |searches, one channel map at a time; spectral=1D |
|                            |            |            |searches, one spectrum at a time (this is        |
|                            |            |            |actually a Duchamp parameter)                    |
+----------------------------+------------+------------+-------------------------------------------------+
|Selavy.thresholdPerWorker   |bool        |false       |If true, each worker's subimage sets its own     |
|                            |            |            |threshold.                                       |
+----------------------------+------------+------------+-------------------------------------------------+


Source fitting
--------------

For continuum images, we have implemented the ability to fit 2D Gaussian components to detected sources. Note that no such facility yet exists for 3D cubes.

Setup
~~~~~

The fitting itself is done by the *fitGaussian* function from the *casacore* package. This returns a set of parameters for each Gaussian: peak flux, x-pixel centre, y-pixel centre, major axis FWHM, axial ratio, position angle.

The pixels that are used in the fit are chosen in one of two ways. If **Selavy.fitJustDetection=true**, then only those pixels that are detected are used in the fit - no "background" pixels are used. This avoid confusion with possible neighbouring sources.

Alternatively, if **Selavy.fitJustDetection=false**, the fitting is done using all pixels in a box surrounding the detection. This box is defined by padding a border of a minimum number of pixels (defined by the **boxPadSize** parameter) in all directions around the object (moving out from the extremities of the detected pixels). Only the pixels lying within this box are used in the fit. This approach is more consistent with the approach used in the FIRST survey.


Noise determination
~~~~~~~~~~~~~~~~~~~

The noise level (ie. the standard deviation of the noise background) is used by the fitting function to weight the fitting function and obtain the chi-squared value. This noise level is not that used by the detection algorithm, but rather one obtained by finding the median absolute deviation from the median (MADFM) in a square box centred on the detection's peak pixel. The side length of the box is governed by the **noiseBoxSize** parameter, which defaults to 101 pixels. Note that it is different to the box mentioned above. The MADFM is converted to an equivalent rms for a Gaussian noise distribution (by dividing by 0.674888). 

It is possible to do the fit without calculating the noise level, by seeting **useNoise = false**. This sets the pixel sigma value to 1 for each pixel, effectively removing the noise from the chi-squared calculation.



Initial estimation of parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The *fitGaussian* function requires an initial estimate of the parameters. The following recursive algorithm provides a list of sub-components (being distinct peaks within a detected object):

* Using the detected object, define a set of parameters: peak flux and location are obvious, while the major & minor axes & position angle are obtained from Duchamp algorithms, using a separate detection process at half the peak flux (to get the full width at half maximum).
* Define a set of sub-thresholds (the number of these is given by the **numSubThresholds** parameter) spaced either linearly or logarithmically between the peak flux and the detection threshold (the separation between sub-thresholds is constant in either log or linear space depending on whether the input parameter **logarithmicThresholds** is true or false). 
* For each threshold, search the box surrounding the object, and record the number of separate detections.

 - If there is more than one separate object, call the getSubComponentList function on each of these and add the result to the vector list.

* When you reach the final threshold, add the initial set of parameters to the vector list and return the vector list.

If the Gaussian fitting fails to provide a good fit, these initial estimates can be returned as the results, with a flag indicating they are only estimates. Whether this is done is governed by the parameter **useGuessIfBad**, which defaults to **true**. If an estimate is reported in the results output, the final column *Guess?* will take the value 1.

Fitting
.......

The list of subcomponents is used to define the intial guess of parameters. The number of Gaussians is between 1 and **maxNumGauss**, and the subcomponents are assigned to the Gaussian components in order of their peak flux. If there are more Gaussians needed than there are subcomponents, we simply cycle through the list.

The fitting is done by casacore's *fitGaussian* function. The fit is repeated a further two times, each time using the output of the previous fit as the initial guess. This results in a slight refinement of the fit, usually (but not always!) improving the chi-squared value.

The parameters that are fitted to the data are defined by the **fitType** parameter. This can take three possible values:

* full: All six parameters of the Gaussian are free to be fitted to the data.
* psf: Only the position and height of the Gaussian are fitted. The size & shape are fixed to match the beam size, taken from the image header (or the beamSize parameter if the image header does not have the beam information).
* shape: Only the position and shape of the Gaussian are fitted. The height is fixed to match the peak pixel flux of the object.

All types can be given in vector format to the **fitType** parameter. In this case, all listed types of fits are done, and the best result (judged by the reduced chi-squared value) is chosen as the best fit. This means that if the best fit for the "full" case is a beam-sized Gaussian, the fit from the "psf" case will be chosen as it has more degrees of freedom and so a lower reduced chi-squared.


Accepting the fit
.................

The fit is accepted according to a list of criteria, that follow those used in the FIRST survey (`Becker, White & Helfand 1995`_). These are:

* The fit must have converged.
* The chi-squared value is examined in one of two ways. The second method is used provided the **chisqConfidence** parameter is between 0 and 1. Otherwise (the default case), the first method is used.

 - The reduced chi-squared is compared to the **maxReducedChisq** parameter, and accepted if smaller. (Here we define ``rchisq = chisq / (npix - numGauss*nfree - 1)``, where *nfree* is the number of free parameters : *See below for discussion*)
 - The chi-squared value and the number of degrees of freedom are used to calculate the probability of a chi-squared-distributed parameter having the given value or less, and compared to the **chisqConfidence** level. For numbers of degrees of freedom greater than 343, computational requirements mean this is approximated by requiring the reduced chi-squared to be less than 1.2.

* The centre of each component must be inside the box
* The separation between any pair of components must be more than 2 pixels
* The flux of each component must be positive and more than half the detection threshold
* No component's peak flux can exceed twice the highest pixel in the box.
* The sum of the integrated fluxes of all components must not be more than twice the total flux in the box.

The results of each of these tests is printed to the log as a 1 (pass) or a 0 (fail).

The default behaviour is to do the fitting using one through to the maximum number of Gaussians, then choose the best fit to be the one that passes all the above criteria and has the lowest reduced chi-squared value. An alternative approach is to set the parameter **stopAfterFirstGoodFit = true**. This will stop fitting after the first acceptable fit is found (starting with a single Gaussian). This way, multiple Gaussians are fitted only if fewer Gaussians do not give an acceptable fit. (NB - this parameter is **false** by default.)

.. _Becker, White & Helfand 1995: http://adsabs.harvard.edu/abs/1995ApJ...450..559B

A note on the reduced chi-squared
.................................

The expression used to calculate the reduced chi-squared as shown above is fine if the pixels are independent. However, this is not the case for radio data, where neighbouring pixels are correlated due to the finite beam size. It is not immediately obvious what the correct way to estimate the reduced chi-squared is. It may be that, formally, a different metric should be used in assessing the goodness-of-fit (since an underlying assumption of the chi-squared test is that the pixels are independent).

Note that, leaving aside the formal requirements of the statistical test, this is primarily a problem when comparing different successful fits that have different numbers of Gaussians. The determination of the best fit for a given number of Gaussians should not be affected (although the second of our acceptance criteria might have to change).

Parameters for fitting
......................

*Note* that from Selavy version 2.1 (12 December 2012), the **doFit** and **fitJustDetection** parameters are now hierarchically placed under **Selavy.Fitter**. Providing **Selavy.doFit** and **Selavy.fitJustDetection** will still work, but a warning message is provided. This check will likely be removed down the track...

+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|*Parameter*                          |*Type*           |*Default*   |*Description*                                                                            |
+=====================================+=================+============+=========================================================================================+
|Selavy.distribFit                    |bool             |true        |If true, the edge sources are distributed by the master node to the workers for          |
|                                     |                 |            |fitting. If false, the master node does all the fitting.                                 |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.doFit                  |bool             |false       |Whether to fit Gaussians to the detections -- necessary if you want to do the quality    |
|                                     |                 |            |evaluation                                                                               |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.fitJustDetection       |bool             |false       |Whether to use just the detected pixels in finding the fit. If false, a rectangular box  |
|                                     |                 |            |is used                                                                                  |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.fitTypes               |vector<string>   |[full,psf]  |A vector of labels for the types of fit to be done. The input format needs to be *a      |
|                                     |                 |            |comma-separated list enclosed by square brackets* (as in the default). There are two     |
|                                     |                 |            |default options: "full", where all 6 parameters in the Gaussian are fitted, and "psf",   |
|                                     |                 |            |where the major&minor axes and the position angle are kept fixed to the beam size. There |
|                                     |                 |            |is also a third option, "shape", where the location and shape are fitted, but the height |
|                                     |                 |            |of the Gaussian is kept at the object's peak flux value. The "shape" option needs to be  |
|                                     |                 |            |specifically requested.                                                                  |
|                                     |                 |            |                                                                                         |
|                                     |                 |            |                                                                                         |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.maxNumGauss            |int              |4           |The maximum number of Gaussians to fit to a single detection                             |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.boxPadSize             |int              |3           |A border of at least this size is added around the detection to create a rectangular box |
|                                     |                 |            |in which the fitting is done.                                                            |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.maxReducedChisq        |float            |5.          |The maximum value for the reduced chi-squared for a fit to be acceptable.                |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.chisqConfidence        |float            |-1.         |A probability value, between 0 and 1, used as a confidence level for accepting the       |
|                                     |                 |            |chi-squared value. If outside this range of values (as is the default), the test is done |
|                                     |                 |            |with the reduced chi-squared value, using the **maxReducedChisq** parameter.             |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.maxRMS                 |float            |1.          |The value that is passed to the FitGaussian::fit() function.                             |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.useNoise               |bool             |true        |Whether to measure the noise in a box surrounding the object and use that as the sigma   |
|                                     |                 |            |value for each point in the fit. Setting to false has the effect of setting the sigma to |
|                                     |                 |            |one for each point.                                                                      |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.noiseBoxSize           |int              |101         |The side length of a box centred on the peak pixel that is used to estimate the noise    |
|                                     |                 |            |level (ie. the rms) for a source: this is used for the fitting.                          |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.minFitSize             |int              |3           |The minimum number of pixels that an object has for it to be fit.                        |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.numSubThresholds       |int              |20          |The number of levels between the detection threshold and the peak that is used to search |
|                                     |                 |            |for subcomponents (these are used for initial guesses of the locations of Gaussian       |
|                                     |                 |            |components).                                                                             |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.logarithmicThresholds  |bool             |true        |Whether the sub-thresholds should be evenly spaced in log-space (true) or linear-space   |
|                                     |                 |            |(false)                                                                                  |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.maxRetries             |int              |0           |The maximum number of retries used by the fitting routine (ie. the maxRetries parameter  |
|                                     |                 |            |for casa::FitGaussian::fit()).                                                           |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.criterium              |double           |0.0001      |The convergence criterium for casa::FitGaussian::fit() (this does not seem to be used in |
|                                     |                 |            |the fitting).                                                                            |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.maxIter                |int              |1024        |The maximum number of iterations in the fit.                                             |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.stopAfterFirstGoodFit  |bool             |false       |Whether to stop the fitting when an acceptable fit is found, without considering fits    |
|                                     |                 |            |with more Gaussian components.                                                           |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+
|Selavy.Fitter.useGuessIfBad          |bool             |true        |Whether to print the initial estimates in the case that the fitting fails                |
|                                     |                 |            |                                                                                         |
+-------------------------------------+-----------------+------------+-----------------------------------------------------------------------------------------+

Missing features (from Duchamp)
-------------------------------

There are a range of features available in the standalone Duchamp package that are *not* implemented in the ASKAPsoft version. If these are requested in the parameter set, a warning message will be written to the log explaining that it is not being used and why. The missing features are summarised here:

* Graphics - pgplot is not included in the ASKAPsoft code tree, so no graphical output is done.
* FDR method - the false discovery rate method is not implemented yet. This method uses all the data to find the optimum threshold, and it is not completely clear how this works in the distributed case, when a given worker only sees part of the data. It could potentially be included in the variable-threshold option, but would be very expensive computationally.
* The baseline removal option in Duchamp has not been implemented. It requires extra work to pass the baseline values over the distributed data, and has not been a priority to implement.
* The various options to save new forms of the image (smoothed array, reconstructed array) and to read them back in have not been implemented - this has not been a priority. The mask output has been enabled, but only for the serial case.
* Similarly, the option of reprocessing a source list (via the Duchamp **usePrevious** parameter) has not been implemented.
* Lastly, there is no ability to write text versions of the spectra of detected sources.

Output files
------------

Standard Duchamp output
~~~~~~~~~~~~~~~~~~~~~~~

Standard Duchamp provides for flexibility in naming the output files it generates. For the ASKAP implementation, these are kept fixed. They are summarised here:

* *selavy-results.txt* - the list of detected sources and their parameters. Also includes (if **flagSeparateHeader=false**) a summary of the input parameters.
* *selavy-results.hdr* - if **flagSeparateHeader=true**, this contains just the input parameter summary from the results file.
* *selavy-results.ann* - a Karma annotation file, showing the location of detected sources. This is produced when **flagKarma=true**, which is the default (contrary to standard Duchamp behaviour)
* *selavy-results.reg* - a DS9 region file, showing the location of detected sources. This is produced when **flagDS9=true**, which is the default (contrary to standard Duchamp behaviour)
* *selavy-Logfile-Master.txt* / *selavy-Logfile-?.txt* - the logfiles, showing lists of intermediate detections (before the final merging), as well as pixel-level details on the final detection list. The first case is for the master node in a parallel-processing system, while the latter are for the workers (or the sole process in a serial system). The '?' represents the worker number, starting at 0. Only the master file (or selavy-Logfile-0.txt for the serial case) has the pixel-level details of the final detections. 
* *selavy-results.xml* - a VOTable of the final list of detections. This is produced when **flagVOT=true** (*not* the default).

ASKAP-specific output
~~~~~~~~~~~~~~~~~~~~~

The following files are produced as a result of the new features implemented in the ASKAP source finder:

* *selavy-SubimageLocations.ann* - a Karma annotation file showing the locations of the subimages used (see "Distributed Processing" section above)
* *selavy-fitResults.txt* - the final set of results from the Gaussian fitting. The format of the file is as follows below. *F_int* and *F_peak* are as calculated by the Duchamp code, and *F_int(fit)* and *F_pk(fit)* are from the fitted Gaussians. Alpha and Beta are the spectral index and spectral curvature terms - these are only provided when examining a Taylor term image. *Maj*, *Min* and *P.A.* are the major and minor FWHMs and the position angle of the fitted Gaussian, quoted for both the fit and the fit deconvolved by the beam. The goodness of fit is indicated by the chi-squared and RMS(fit) values, while RMS(image) gives the local noise surrounding the object. Nfree(fit) is the number of free parameters in the fit, and NDoF(fit) is the number of degrees of freedom. Npix(fit) is the number of pixels used in doing the fit, and Npix(obj) is the number of pixels in the object itself (ie. detected pixels). If no fit was made, all the *(fit)* values are set to zero. 
* *selavy-fitResults.xml* - a VOTable version of the fit results. This is always produced whenever selavy-fitResults.txt is produced.
* *selavy-fitResults.ann* - a Karma annotation file showing the fitting results (each Gaussian component is indicated by an ellipse given by the major & minor axes and position angle of the component)
* *selavy-fitResults.boxes.ann* - a Karma annotation file showing the boxes used for the Gaussian fitting (if used). See above discussion for details.

Logging
~~~~~~~

The final output file is the log (not to be confused with the selavy-Logfile-* files described above). This is the set of log messages (information, warning, errors) that describe the progress of the program. Each log message is tagged by the level of the message, its origin & machine/host, and date/time. These can be very large, particularly in the distributed case when Gaussian fitting is done. The main use for this file is to ensure that all steps of the algorithm proceed correctly, to identify problems, or to keep track of the time taken by various parts. 

A typical line from the log might look like this:
<pre>
INFO  analysis.parallelanalysis (5, minicp04) [2011-03-02 12:57:58,438] - Worker #5: Setting threshold to be 0.0153364
</pre>
The different parts of the message are:
* INFO - the level of the message: INFO, WARN or FATAL
* analysis.parallelanalysis - from which software element does the log message originate
* (5, minicp04) - the process number (0=master process, >0 = worker) and the machine it is running on.
* [2011-03-02 12:57:58,438] - date & time of log message
* and the rest is the actual message

Note that if you want to see all messages for a given worker, you could do something like
<pre>
grep "(3, " logfile.log
</pre>
This is often necessary to disentangle the log streams of the different nodes.

Note also that the log file may also include information not in this form, that has just been written to stdout by some part of the code.

Memory considerations
---------------------

Finally, a note about memory usage. The large datasets such as the spectral-line ASKAP simulations present challenges for the processing due to the large amount of memory required to read the images. The distributed processing allows the user to split up a large image across multiple nodes, thereby reducing the memory impact. However, this is still limited, and the user needs to be aware of how the memory is used. 

Each worker (but not the master) will allocate an array of floats the size of the subsection it reads in. There will be additional allocation for the following major items (plus a host of more minor stuff):

* a reconstructed or smoothed array of the same size, should these be requested
* the weights image, of the same size, should this be used for the scaling of the threshold
* lists of detected pixels on the master (x,y,z position + flux) - for large amounts of sources this can be quite large

The user needs to consider how the memory will be apportioned for a given distribution scheme, and what the machine in question is capable of. The *minicp* cluster that the source finder service is run on has 8 compute nodes, each with 2 dual-core processors and 12GB of memory. So you can have 4 processes running at once on a node, with an average of 3GB/process available. Running a job in serial mode (ie. a single cpu) actually requests an entire node, so that you have 12 GB available. This is to stop one job stealing all the memory from another.
