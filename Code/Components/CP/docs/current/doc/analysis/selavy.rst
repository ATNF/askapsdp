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

The following table lists parameters that control different modes of Selavy. They are not switched on by default

+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|*Parameter*         |*Type*              |*Default*           |*Description*                                                    |
+====================+====================+====================+=================================================================+
|flagATrous          |bool                |false               |Use the a trous wavelet reconstruction algorithm prior to        |
|                    |                    |                    |source-finding. See the Preprocessing_ page for details.         |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|flagSmooth          |bool                |false               |Use spectral or spatial smoothing prior to source-finding. See   |
|                    |                    |                    |the Preprocessing_ page for details.                             |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|recon2D1D           |bool                |false               |Use the 2D1D wavelet reconstruction algorithm (provided by       |
|                    |                    |                    |WALLABY). See the Preprocessing_ page for details.               |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|optimiseMask        |bool                |false               |Whether to use the mask optimisation algorithm to optimally      |
|                    |                    |                    |increase the size of each object. See the Postprocessing_ page   |
|                    |                    |                    |for details.                                                     |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|extractSpectra      |bool                |false               |Extract a spectrum (to a CASA image) for each detected source.   |
|                    |                    |                    |See the Extraction_ page for details.                            |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|extractNoiseSpectra |bool                |false               |Extract a noise spectrum (to a CASA image) for each detected     |
|                    |                    |                    |source. See the Extraction_ page for details.                    |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+
|Fitter.doFit        |bool                |false               |Fit Gaussian components to objects detected in a two-dimensional |
|                    |                    |                    |image. See the Postprocessing_ page for details.                 |
+--------------------+--------------------+--------------------+-----------------------------------------------------------------+

.. _Preprocessing: preprocessing.html
.. _Postprocessing: postprocessing.html
.. _Extraction: extraction.html


Distributed processing
----------------------

The primary innovation in the ASKAPsoft implementation has been to allow distributed processing of images, to test the likely functionality of pipeline processing. The image is split up into subimages according to a user-specified scheme (the user provides the number of subdivisions in the x-, y- and z-directions). Neighbouring subimages can be overlapped by a certain amount (which may be desirable, particularly in the case of variable thresholds - see below). 

Processing is performed under a master-worker framework, where a single master process coordinates the processing, and each worker handles a single subimage. Each of these subimages is searched independently, then the worker sends the list of detected sources to the master process. Once the master has accumulated the full set of detected sources, objects near the overlap regions are merged (if necessary) and have their parameters recalculated. The results are then written out.


Variable thresholds
-------------------

The Duchamp package uses a single detection threshold for the entire image being searched. However, if the sensitivity varies across the field, this will either mean some regions are not searched as deep as they could be and/or some are searched too deeply, resulting in too many spurious detections. The ASKAP implementation deals with this in one of two ways.

The first is to use a weights image, such as that produced by the ASKAPsoft imager (and included in most of the ASKAP simulations), to scale the image according to the sensitivity. In practice, this takes the square root of the normalised weights and divides this into the pixel values. This has the effect of scaling down the low-sensitivity regions of the image, making it less likely that they present many spurious detections. The weights image is specified via **Selavy.weightsimage**. The detection thresholds are provided in the usual fashion. The pixel values are only affected for the detection phase - parameter calculations are *not* affected.

The alternative is to impose a signal-to-noise threshold based on the *local* noise surrounding the pixel in question. This threshold then varies from pixel to pixel based on the change in the local noise. This mode is turned on using the **Selavy.doMedianSearch** parameter, which default to false.

This "local" level is estimated by measuring the median and median absolute deviation from the median of pixels within a box centred on the pixel in question. An array is thus built up containing the signal-to-local-noise values for each pixel in the image, and this array is then searched with a SNR threshold (**Selavy.snrCut**) and, if necessary, grown to a secondary SNR threshold (**Selavy.growthCut**). 

The searching can be done either spatially or spectrally, and this affects how the SNR values are calculated. If spatially (the default), a 2D sliding box filter is used to find the local noise. If spectrally, only a 1D "box" is used. Note that the edges (ie. all pixels within the half box width of the edge) are set to zero, and so detections will not be made there. This probably won't affect the 2D case, as often the edges of the field have poor sensitivity (certainly the ASKAP simulations mostly have a padding region around the edge), but in the 1D case this will mean the loss of the first & last channels. The choice between 2D and 1D is made with the **Selavy.searchType** parameter (which actually comes out of the Duchamp package).

When run on a distributed system as above, this processing is done at the worker level. Note that having an overlap between workers of at least the half box width will give continuous coverage (avoiding the aforementioned edge problems). The amount of processing needed increases quickly with the size of the box, due to the use of medians, particularly for the 2D case. 

A final option for varying the threshold spatially is to use a different threshold for each worker. In this scenario, switched on by setting **thresholdPerWorker = true**, each worker finds its own threshold based on the noise within it, using the **snrCut** signal-to-noise ratio threshold. No variation of the threshold *within* a worker is done, so you get discrete jumps in the threshold at worker boundaries. Use of the overlap can mitigate this. This mode was implemented more as an experiment than out of any expectation it would be useful, and limited trials indicate it's probably not much use. For completeness we include the parameter here. 

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
::

 INFO  analysis.parallelanalysis (5, minicp04) [2011-03-02 12:57:58,438] - Worker #5: Setting threshold to be 0.0153364

The different parts of the message are:

* INFO - the level of the message: DEBUG, INFO, WARN, ERROR or FATAL
* analysis.parallelanalysis - from which software module does the log message originate
* (5, minicp04) - the process number (0=master process, >0 = worker) and the machine it is running on.
* [2011-03-02 12:57:58,438] - date & time of log message
* and the rest is the actual message

Note that if you want to see all messages for a given worker, you could do something like ``grep "(3, " logfile.log``. This is often necessary to disentangle the log streams of the different nodes. Note also that the log file may also include information not in this form, that has just been written to stdout by some part of the code.
