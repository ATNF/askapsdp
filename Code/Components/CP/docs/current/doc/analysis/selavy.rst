Selavy Basics
=============

Summary
-------

Selavy is the ASKAP implementation of the source-finding algorithms of Duchamp (publicly available at the `Duchamp website`_). 

Duchamp is a source-finder designed for three-dimensional datasets (such as HI spectral-line cubes), but is easily applied to two- or one-dimensional data. There is an extensive user's guide available on the Duchamp website that provides details about the algorithms. Users are also referred to the Duchamp journal paper `Whiting (2012), MNRAS 421, 3242`_ for further details and examples. The name 'Duchamp' comes from `Marcel Duchamp`_, the artist who pioneered the art of the "readymade", or "found object". 

Selavy uses the core Duchamp algorithms, and adds new features as dictated by the requirements of ASKAP processing. See `Whiting & Humphreys (2012), PASA 29, 371`_
for descriptions and some benchmarks. The new features include support for distributed processing, Gaussian fitting to two-dimensional images, variable-threshold detection, and additional pre- and post-processing algorithms specified by the Survey Science Teams. The name 'Selavy' comes from `Rrose Selavy`_, a pseudonym of Marcel Duchamp.

 .. _Duchamp website: http://www.atnf.csiro.au/people/Matthew.Whiting/Duchamp
 .. _Whiting (2012), MNRAS 421, 3242: http://onlinelibrary.wiley.com/doi/10.1111/j.1365-2966.2012.20548.x/full
 .. _Whiting & Humphreys (2012), PASA 29, 371: http://www.publish.csiro.au/paper/AS12028.htm 
 .. _Marcel Duchamp: http://en.wikipedia.org/wiki/Marcel_Duchamp
 .. _Rrose Selavy: http://en.wikipedia.org/wiki/Rrose_Selavy

Basic Execution and Control Parameters
--------------------------------------

This section summarises the basic execution of Selavy, with control of specific types of processing discussed in later sections. The default approach of Selavy is to follow the Duchamp method of applying a single threshold, either a flux value (specified by the user) or a signal-to-noise level, to the entire image. Pixels above this threshold are grouped together in objects - they may be required to be contiguous, or they can be separated by up to some limit. Detected objects may be "grown" to a secondary (lower) threshold, to include faint pixels without having to search to a faint level. 

One of the aims of Selavy is to provide distributed processing support, and details on how this is done can be found `below`_. To get a single SNR threshold in this case, the noise stats are calculated on a subimage basis and combined to find the average noise across the image. Selavy also permits variable detection thresholds to be determined in several ways - these are described on the `Thresholds`_ page.

The input image can be a FITS format image or a CASA image (as is produced by the ASKAPsoft imaging pipeline). The code to read CASA images is new: it extracts the flux, WCS and metadata information and stores them in the classes used by the Duchamp code. While the Duchamp software is able to read FITS images, this has issues with very large cubes depending on the memory of the system being used - sometimes even the getMetadata() function would break (e.g. when subsections were being requested) when the number of pixels in the subsection was more than memory could hold. For this reason, the CASA code is default, unless the **useCASAforFITS** parameter is set to false.

.. _`below`: selavy.html#distributed-processing
.. _`Thresholds`: thresholds.html

General parameters
~~~~~~~~~~~~~~~~~~

The following table lists the basic parameters governing the searching. All parameters listed here should have a **Selavy.** prefix (ie. **Selavy.image**). 

All parameters from Duchamp can be provided in an input parameter set, although some are not implemented in Selavy. A list of these and brief explanations can be found on the `Duchamp Exclusions`_ page.

.. _`Duchamp Exclusions`: duchampExclusions.html

+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|*Parameter*            |*Type*        |*Default*            |*Description*                                                                           |
+=======================+==============+=====================+========================================================================================+
|image                  |string        |none                 |The input image - can also use **imageFile** for consistency with Duchamp parameters    |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|useCASAforFITS         |bool          |true                 |Whether to use the casacore functionality for FITS images (if false, the native Duchamp |
|                       |              |                     |code is used - see discussion above)                                                    |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|searchType             |string        |spatial              |How the searches are done: in 2D channel maps ("spatial") or in 1D spectra ("spectral") |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|snrCut                 |float         |4.                   |Threshold value, in multiples of the rms above the mean noise level                     |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|threshold              |float         |*no default*         |Threshold value in units of the image (default value means snrCut will be used instead) |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagGrowth             |bool          |false                |Whether to grow detections to a lower threshold                                         |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|growthCut              |float         |3.                   |Signal-to-noise ratio to grow detections down to                                        |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|growthThreshold        |float         |0.                   |Threshold value to grow detections down to (takes precedence over **growthCut**)        |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagNegative           |bool          |false                |Whether to invert the cube and search for negative features                             |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagRobustStats        |bool          |true                 |Whether to use robust statistics when evaluating image noise statistics.                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|minPix                 |int           |2                    |Minimum number of pixels allowed in a detection                                         |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|minChannels            |int           |3                    |Minimum number of channels allowed in a detection                                       |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|minVoxels              |int           |minPix + minChannels |Minimum number of voxels allowed in a detection. Will be *at least* minPix + minChannels|
|                       |              |+ 1                  |+ 1, but can be higher.                                                                 |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|maxPix                 |int           |-1                   |Maximum number of pixels allowed in a detection. No check is made if value is negative. |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|maxChannels            |int           |-1                   |Maximum number of channels allowed in a detection . No check is made if value is        |
|                       |              |                     |negative.                                                                               |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|maxVoxels              |int           |-1                   |Maximum number of voxels allowed in a detection. No check is made if value is negative. |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagAdjacent           |bool          |true                 |When merging sources, whether sources need to be adjacent to be merged                  |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|threshSpatial          |float         |3.                   |When flagAdjacent=false, this is the spatial threshold (in pixels) within which objects |
|                       |              |                     |are combined                                                                            |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|threshVelocity         |float         |7.                   |When flagAdjacent=false, this is the frequency threshold (in channels) within which     |
|                       |              |                     |objects are combined                                                                    |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagRejectBeforeMerge  |bool          |false                |A flag indicating whether to reject sources that fail to meet the minimum size criteria |
|                       |              |                     |**before** the merging stage. Default behaviour is to do the rejection last.            |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagTwoStageMerging    |bool          |true                 |A flag indicating whether to do an initial merge of newly-detected sources into the     |
|                       |              |                     |source list as they are found. If false, new sources are simply added to the end of the |
|                       |              |                     |list for later merging.                                                                 |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagSubsection         |bool          |false                |Whether to look at just a subsection of the given image                                 |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|subsection             |string        |*full image*         |A subsection string, e.g. **[1001:2000, 50:1572, *, *]** - only these pixels will be    |
|                       |              |                     |examined                                                                                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagStatSec            |bool          |false                |Whether to use a subsection of the image to calculate the image statistics              |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|statsec                |string        |*full image*         |A subsection string indicating the pixel region to be used to calculate statistics      |
|                       |              |                     |(mean, rms,...)                                                                         |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flaggedChannels        |string        |*no default*         |A comma-separated list of channels and channel ranges (for instance, 1,2,5-9,13) that   |
|                       |              |                     |should be ignored for the purposes of detecting objects. Replaces the old (for Duchamp  |
|                       |              |                     |versions <1.5) flagMW/minMW/maxMW parameters.                                           |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|beamArea               |float         |10.                  |The area of the beam in *pixels*. This parameter is only used when the image does not   |
|                       |              |                     |provide beam information. When this is used, a circular beam is assumed.                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|beamFWHM               |float         |-1.                  |The FWHM of the beam in *pixels*. This parameter is only used when the image does not   |
|                       |              |                     |provide beam information. When this is used, a circular beam is assumed. This value     |
|                       |              |                     |takes precedence over **beamArea** but is ignored if negative (the default).            |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|spectralUnits          |string        |*no default*         |The units desired for the spectral axis. If no value is given, the units in the image   |
|                       |              |                     |header are used.                                                                        |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|spectralType           |string        |*no default*         |An alternative WCS type that the spectral axis is to be expressed in. If no value is    |
|                       |              |                     |given, the type held by the image header is used. The specification should conform to   |
|                       |              |                     |the standards described in `Greisen et al (2006)`_, although it is possible to provide  |
|                       |              |                     |just the first four letters (the 'S-type', e.g. 'VELO').                                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|restFrequency          |float         |-1.                  |If provided, this will be used in preference to the rest frequency given in the image   |
|                       |              |                     |header. If not provided, the image header value will be used if required.               |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+

 .. _`Greisen et al (2006)`: http://adsabs.harvard.edu/abs/2006A%26A...446..747G

Control switches
~~~~~~~~~~~~~~~~

The following table lists parameters that control different modes of Selavy. They are not switched on by default

+---------------------+---------+----------+-------------------------------------------------------------------+
|*Parameter*          |*Type*   |*Default* |*Description*                                                      |
+=====================+=========+==========+===================================================================+
|flagATrous           |bool     |false     |Use the à trous wavelet reconstruction algorithm prior to          |
|                     |         |          |source-finding. See the Preprocessing_ page for details.           |
+---------------------+---------+----------+-------------------------------------------------------------------+
|flagSmooth           |bool     |false     |Use spectral or spatial smoothing prior to source-finding. See the |
|                     |         |          |Preprocessing_ page for details.                                   |
+---------------------+---------+----------+-------------------------------------------------------------------+
|recon2D1D            |bool     |false     |Use the 2D1D wavelet reconstruction algorithm (provided by         |
|                     |         |          |WALLABY). See the Preprocessing_ page for details.                 |
+---------------------+---------+----------+-------------------------------------------------------------------+
|WeightScaling        |bool     |false     |If true, scale the fluxes in the image by the normalised weights,  |
|                     |         |          |to remove sensitivity varations. See the Thresholds_ page for      |
|                     |         |          |details.                                                           |
+---------------------+---------+----------+-------------------------------------------------------------------+
|VariableThreshold    |bool     |false     |If true, use a sliding box to find the noise local to a pixel and  |
|                     |         |          |set the (spatially-varying) detection threshold accordingly. See   |
|                     |         |          |the Thresholds_ page for details.                                  |
+---------------------+---------+----------+-------------------------------------------------------------------+
|optimiseMask         |bool     |false     |Whether to use the mask optimisation algorithm to optimally        |
|                     |         |          |increase the size of each object. See the Postprocessing_ page for |
|                     |         |          |details.                                                           |
+---------------------+---------+----------+-------------------------------------------------------------------+
|extractSpectra       |bool     |false     |Extract a spectrum (to a CASA image) for each detected source. See |
|                     |         |          |the Extraction_ page for details.                                  |
+---------------------+---------+----------+-------------------------------------------------------------------+
|extractNoiseSpectra  |bool     |false     |Extract a noise spectrum (to a CASA image) for each detected       |
|                     |         |          |source. See the Extraction_ page for details.                      |
+---------------------+---------+----------+-------------------------------------------------------------------+
|Fitter.doFit         |bool     |false     |Fit Gaussian components to objects detected in a two-dimensional   |
|                     |         |          |image. See the Postprocessing_ page for details.                   |
+---------------------+---------+----------+-------------------------------------------------------------------+

.. _Preprocessing: preprocessing.html
.. _Thresholds: thresholds.html
.. _Postprocessing: postprocessing.html
.. _Extraction: extraction.html


Distributed processing
----------------------

Description
~~~~~~~~~~~

The primary innovation in the ASKAPsoft implementation has been to allow distributed processing of images, to test the likely functionality of pipeline processing. The image is split up into subimages according to a user-specified scheme (the user provides the number of subdivisions in the x-, y- and z-directions). Neighbouring subimages can be overlapped by a certain amount (which may be desirable, particularly in the case of variable thresholds - see below). 

Processing is performed under a master-worker framework, where a single master process coordinates the processing, and each worker handles a single subimage. Each of these subimages is searched independently, then the worker sends the list of detected sources to the master process. Once the master has accumulated the full set of detected sources, objects near the overlap regions are merged (if necessary) and have their parameters recalculated. The results are then written out.

Distributed processing parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|*Parameter*            |*Type*        |*Default*            |*Description*                                                                           |
+=======================+==============+=====================+========================================================================================+
|nsubx                  |int           |1                    |The number of subdivisions in the x-direction when making the subimages.                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|nsuby                  |int           |1                    |The number of subdivisions in the y-direction when making the subimages.                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|nsubz                  |int           |1                    |The number of subdivisions in the z-direction when making the subimages.                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|overlapx               |int           |0                    |The number of pixels of overlap between neighbouring subimages in the x-direction       |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|overlapy               |int           |0                    |The number of pixels of overlap between neighbouring subimages in the y-direction       |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|overlapz               |int           |0                    |The number of pixels of overlap between neighbouring subimages in the z-direction       |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|subimageAnnotationFile |string        |""                   |The filename of a Karma annotation file that is created to show the boundaries of the   |
|                       |              |                     |subimages (see description below). If empty, no such file is created.                   |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+



Output files
------------

Standard Duchamp output
~~~~~~~~~~~~~~~~~~~~~~~

Standard Duchamp provides for flexibility in naming the output files it generates. For the ASKAP implementation, these are kept fixed. They are summarised here, listed by the parameter name with the default value in square brackets.:

* **resultsFile** [selavy-results.txt] - the list of detected sources and their parameters. Also includes (if **flagSeparateHeader=false**, the default case) a summary of the input parameters.
* **headerFile** [selavy-results.hdr] - if **flagSeparateHeader=true**, this contains just the input parameter summary from the results file.
* **karmaFile** [selavy-results.ann] - a Karma annotation file, showing the location of detected sources. This is produced when **flagKarma=true**, which is the default (contrary to standard Duchamp behaviour)
* **ds9File** [selavy-results.reg] - a DS9 region file, showing the location of detected sources. This is produced when **flagDS9=true**, which is the default (contrary to standard Duchamp behaviour)
* **logFile** [selavy-Logfile.txt / selavy-Logfile-Master.txt / selavy-Logfile-?.txt] - the logfiles, showing lists of intermediate detections (before the final merging), as well as pixel-level details on the final detection list. The first default listed is the default when running serial processing. The other two come from the distributed-processing case. In this case, the parameter's value has either '-Master' or '-?' (where ? is replaced by the worker number, starting at 0) inserted before the suffix, or at the end if there is no suffix in the name provided. Only the master file (or the sole logfile in the serial case) has the pixel-level details of the final detections. 
* **votFile** [selavy-results.xml] - a VOTable of the final list of detections. This is produced when **flagVOT=true** (*not* the default).

ASKAP-specific output
~~~~~~~~~~~~~~~~~~~~~

The following files are produced as a result of the new features implemented in the ASKAP source finder:

* **subimageAnnotationFile** [selavy-SubimageLocations.ann] - a Karma annotation file showing the locations of the subimages used (see "Distributed Processing" section above)
* **fitResultsFile** [selavy-fitResults.txt] - the final set of results from the Gaussian fitting -- see Fitting_ for details. The format of the file is as follows below. *F_int* and *F_peak* are as calculated by the Duchamp code, and *F_int(fit)* and *F_pk(fit)* are from the fitted Gaussians. Alpha and Beta are the spectral index and spectral curvature terms - these are only provided when examining a Taylor term image. *Maj*, *Min* and *P.A.* are the major and minor FWHMs and the position angle of the fitted Gaussian, quoted for both the fit and the fit deconvolved by the beam. The goodness of fit is indicated by the chi-squared and RMS(fit) values, while RMS(image) gives the local noise surrounding the object. Nfree(fit) is the number of free parameters in the fit, and NDoF(fit) is the number of degrees of freedom. Npix(fit) is the number of pixels used in doing the fit, and Npix(obj) is the number of pixels in the object itself (ie. detected pixels). If no fit was made, all the *(fit)* values are set to zero. A VOTable version of the fit results is also produced, with a .xml suffix. This is always produced whenever selavy-fitResults.txt is produced.
* **fitAnnotationFile** [selavy-fitResults.ann] - a Karma annotation file showing the fitting results (each Gaussian component is indicated by an ellipse given by the major & minor axes and position angle of the component).
* **fitBoxAnnotationFile** [selavy-fitResults.boxes.ann] - a Karma annotation file showing the boxes used for the Gaussian fitting (if used). See Fitting_ for details.

.. _Fitting: postprocessing.html#source-fitting

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


Output-related parameters
~~~~~~~~~~~~~~~~~~~~~~~~~

+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|*Parameter*              |*Type*        |*Default*                   |*Description*                                                                           |
+=========================+==============+============================+========================================================================================+
|verbose                  |bool          |false                       |Controls the verbosity for the Duchamp-specific code. **verbose=true** means more       |
|                         |              |                            |information about the Duchamp functions                                                 |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|pixelCentre              |string        |centroid                    |How the central pixel value is defined in the output catalogues (can take values of     |
|                         |              |                            |'centroid', 'average' or 'peak').                                                       |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|resultsFile              |string        |selavy-results.txt          |The text file holding the catalogue of results. Can also use **outFile** for            |
|                         |              |                            |compatbility with Duchamp.                                                              |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagSeparateHeader       |bool          |false                       |Whether the "header" containing the summary of input parameters should be written to a  |
|                         |              |                            |separate file from the table of results. If produced, it will be called                 |
|                         |              |                            |selavy-results.hdr.                                                                     |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|subimageAnnotationFile   |string        |""                          |The filename of a Karma annotation file that is created to show the boundaries of the   |
|                         |              |                            |subimages (see description below). If empty, no such file is created.                   |
|                         |              |                            |                                                                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagLog                  |bool          |true                        |Produce a Duchamp-style log file, recording intermediate detections (see below). *Note  |
|                         |              |                            |the different default from standard Duchamp.* The workers will produce                  |
|                         |              |                            |selavy-Logfile.%w.txt, (where %w is the worker number, in the usual fashion) and the    |
|                         |              |                            |master will produce selavy-Logfile.Master.txt.                                          |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagVOT                  |bool          |true                        |Produce a VOTable of the results.                                                       |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|votFile                  |string        |selavy-results.txt          |The VOTable containing the catalogue of detections.                                     |
|                         |              |                            |                                                                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagWriteBinaryCatalogue |bool          |true                        |Produce a binary catalogue compatible with Duchamp (that can be loaded into Duchamp     |
|                         |              |                            |along with the image to produce plots of the detections).                               |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|binaryCatalogue          |string        |selavy-catalogue.dpc        |The binary catalogue.                                                                   |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagTextSpectra          |bool          |false                       |Produce a file with text-based values of the spectra of each detection.                 |
|                         |              |                            |                                                                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|spectraTextFile          |string        |selavy-spectra.txt          |The file containing ascii spectra of each detection.                                    |
|                         |              |                            |                                                                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|objectList               |string        |*no default*                |A comma-separated list of objects that will be used for the post-processing. This is    |
|                         |              |                            |inherited from Duchamp, where it can be used to only plot a selection of sources. This  |
|                         |              |                            |is most useful for re-running with a previously-obtained catalogue.  In Selavy, this    |
|                         |              |                            |will only be applied to the spectraTextFile and spectral extraction options (see the    |
|                         |              |                            |`Extraction`_ page for details on the latter).                                          |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagKarma                |bool          |true                        |Produce a Karma annotation plot. *Note the different default from standard Duchamp.*    |
|                         |              |                            |                                                                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|karmaFile                |string        |selavy-results.ann          |The Karma annoation file showing the location of detected objects.                      |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagDS9                  |bool          |true                        |Produce a DS9 region file.  *Note the different default from standard Duchamp.*         |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|ds9File                  |string        |selavy-results.reg          |The DS9 region file showing the location of detected objects.                           |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|flagCasa                 |bool          |true                        |Produce a CASA region file.  *Note the different default from standard Duchamp.*        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|casaFile                 |string        |selavy-results.crf          |The CASA region format file showing the location of detected objects.                   |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|drawBorders              |bool          |true                        |Whether to draw the object borders in the annotation file. If false, only a circle is   |
|                         |              |                            |drawn with radius proportional to the object's size.                                    |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|fitResultsFile           |string        |selavy-fitResults.txt       |The ASCII file containing the results of the Guassian fitting                           |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|fitAnnotationFile        |string        |selavy-fitResults.ann       |A Karma annotation file showing the location, size & shape of fitted components.        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|fitBoxAnnotationFile     |string        |selavy-fitResults.boxes.ann |A Karma annoation file showing the location and size of boxes used in the Gaussian      |
|                         |              |                            |fitting (only produced if Fitter.fitJustDetection = false).                             |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|precFlux                 |int           |3                           |Precision for the flux values in the output files                                       |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|precVel                  |int           |3                           |Precision for the velocity values in the output files                                   |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|precSNR                  |int           |2                           |Precision for the SNR values in the output files                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+
|sortingParam             |string        |vel                         |Which parameter to sort the output list by: x-value, y-value, z-value, ra, dec, vel,    |
|                         |              |                            |w50, iflux, pflux, snr. A - prepended to the parameter reverses the order of the sort.  |
|                         |              |                            |                                                                                        |
+-------------------------+--------------+----------------------------+----------------------------------------------------------------------------------------+

.. _`Extraction`: extraction.html
