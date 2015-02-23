Selavy Basics
=============

Summary
-------

Selavy is the ASKAP implementation of the source-finding algorithms of
Duchamp (publicly available at the `Duchamp website`_).

Duchamp is a source-finder designed for three-dimensional datasets
(such as HI spectral-line cubes), but is easily applied to two- or
one-dimensional data. There is an extensive user's guide available on
the Duchamp website that provides details about the algorithms. Users
are also referred to the Duchamp journal paper `Whiting (2012), MNRAS
421, 3242`_ for further details and examples. The name 'Duchamp' comes
from `Marcel Duchamp`_, the artist who pioneered the art of the
"readymade", or "found object".

Selavy uses the core Duchamp algorithms, and adds new features as
dictated by the requirements of ASKAP processing. See `Whiting &
Humphreys (2012), PASA 29, 371`_ for descriptions and some
benchmarks. The new features include support for distributed
processing, Gaussian fitting to two-dimensional images,
variable-threshold detection, and additional pre- and post-processing
algorithms specified by the Survey Science Teams. The name 'Selavy'
comes from `Rrose Selavy`_, a pseudonym of Marcel Duchamp.

 .. _Duchamp website: http://www.atnf.csiro.au/people/Matthew.Whiting/Duchamp
 .. _Whiting (2012), MNRAS 421, 3242: http://onlinelibrary.wiley.com/doi/10.1111/j.1365-2966.2012.20548.x/full
 .. _Whiting & Humphreys (2012), PASA 29, 371: http://www.publish.csiro.au/paper/AS12028.htm 
 .. _Marcel Duchamp: http://en.wikipedia.org/wiki/Marcel_Duchamp
 .. _Rrose Selavy: http://en.wikipedia.org/wiki/Rrose_Selavy

Basic Execution and Control Parameters
--------------------------------------

This section summarises the basic execution of Selavy, with control of
specific types of processing discussed in later sections. The default
approach of Selavy is to follow the Duchamp method of applying a
single threshold, either a flux value (specified by the user) or a
signal-to-noise level, to the entire image. Pixels above this
threshold are grouped together in objects - they may be required to be
contiguous, or they can be separated by up to some limit. Detected
objects may be "grown" to a secondary (lower) threshold, to include
faint pixels without having to search to a faint level.

One of the aims of Selavy is to provide distributed processing
support, and details on how this is done can be found `below`_. To get
a single SNR threshold in this case, the noise stats are calculated on
a subimage basis and combined to find the average noise across the
image. Selavy also permits variable detection thresholds to be
determined in several ways - these are described on the `Thresholds`_
page.

The input image can be a FITS format image or a CASA image (as is
produced by the ASKAPsoft imaging pipeline). The code to read CASA
images is new: it extracts the flux, WCS and metadata information and
stores them in the classes used by the Duchamp code. While the Duchamp
software is able to read FITS images, this has issues with very large
cubes depending on the memory of the system being used - sometimes
even the getMetadata() function would break (e.g. when subsections
were being requested) when the number of pixels in the subsection was
more than memory could hold. For this reason, the CASA code is
default, unless the **useCASAforFITS** parameter is set to false.

.. _`below`: selavy.html#distributed-processing
.. _`Thresholds`: thresholds.html

General parameters
~~~~~~~~~~~~~~~~~~

The following table lists the basic parameters governing the
searching. All parameters listed here should have a **Selavy.** prefix
(ie. **Selavy.image**).

All parameters from Duchamp can be provided in an input parameter set,
although some are not implemented in Selavy. A list of these and brief
explanations can be found on the `Duchamp Exclusions`_ page.

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
|snrCut                 |float         |4.0                  |Threshold value, in multiples of the rms above the mean noise level                     |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|threshold              |float         |*no default*         |Threshold value in units of the image (default value means snrCut will be used instead) |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|flagGrowth             |bool          |false                |Whether to grow detections to a lower threshold                                         |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|growthCut              |float         |3.0                  |Signal-to-noise ratio to grow detections down to                                        |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|growthThreshold        |float         |0.0                  |Threshold value to grow detections down to (takes precedence over **growthCut**)        |
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
|threshSpatial          |float         |3.0                  |When flagAdjacent=false, this is the spatial threshold (in pixels) within which objects |
|                       |              |                     |are combined                                                                            |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|threshVelocity         |float         |7.0                  |When flagAdjacent=false, this is the frequency threshold (in channels) within which     |
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
|beamArea               |float         |10.0                 |The area of the beam in *pixels*. This parameter is only used when the image does not   |
|                       |              |                     |provide beam information. When this is used, a circular beam is assumed.                |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+
|beamFWHM               |float         |-1.0                 |The FWHM of the beam in *pixels*. This parameter is only used when the image does not   |
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
|restFrequency          |float         |-1.0                 |If provided, this will be used in preference to the rest frequency given in the image   |
|                       |              |                     |header. If not provided, the image header value will be used if required.               |
+-----------------------+--------------+---------------------+----------------------------------------------------------------------------------------+

 .. _`Greisen et al (2006)`: http://adsabs.harvard.edu/abs/2006A%26A...446..747G

Control switches
~~~~~~~~~~~~~~~~

The following table lists parameters that control different modes of
Selavy. They are not switched on by default.

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
|Weights              |bool     |false     |If true, scale the fluxes in the image by the normalised weights,  |
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

The primary innovation in the ASKAPsoft implementation has been to
allow distributed processing of images, to test the likely
functionality of pipeline processing. The image is split up into
subimages according to a user-specified scheme (the user provides the
number of subdivisions in the x-, y- and z-directions). Neighbouring
subimages can be overlapped by a certain amount (which may be
desirable, particularly in the case of variable thresholds - see
below).

Processing is performed under a master-worker framework, where a
single master process coordinates the processing, and each worker
handles a single subimage. Each of these subimages is searched
independently, then the worker sends the list of detected sources to
the master process. Once the master has accumulated the full set of
detected sources, objects near the overlap regions are merged (if
necessary) and have their parameters recalculated. The results are
then written out.

Distributed processing parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|*Parameter*            |*Type*        |*Default*                            |*Description*                                                                           |
+=======================+==============+=====================================+========================================================================================+
|nsubx                  |int           |1                                    |The number of subdivisions in the x-direction when making the subimages.                |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|nsuby                  |int           |1                                    |The number of subdivisions in the y-direction when making the subimages.                |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|nsubz                  |int           |1                                    |The number of subdivisions in the z-direction when making the subimages.                |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|overlapx               |int           |0                                    |The number of pixels of overlap between neighbouring subimages in the x-direction       |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|overlapy               |int           |0                                    |The number of pixels of overlap between neighbouring subimages in the y-direction       |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|overlapz               |int           |0                                    |The number of pixels of overlap between neighbouring subimages in the z-direction       |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+
|subimageAnnotationFile |string        |selavy-SubimageLocations.ann         |The filename of a Karma annotation file that is created to show the boundaries of the   |
|                       |              |                                     |subimages (see description below). If empty, no such file is created.                   |
+-----------------------+--------------+-------------------------------------+----------------------------------------------------------------------------------------+



Output files
------------

Standard Duchamp output
~~~~~~~~~~~~~~~~~~~~~~~

Standard Duchamp provides for flexibility in naming the output files
it generates. They are summarised here, listed by the parameter name
with the default value in square brackets.:

* **resultsFile** [*selavy-results.txt*] - the list of detected
  sources and their parameters. Also includes (if
  **flagSeparateHeader=false**, the default case) a summary of the
  input parameters.
* **headerFile** [*selavy-results.hdr*] - if
  **flagSeparateHeader=true**, this contains just the input parameter
  summary from the results file.
* **karmaFile** [*selavy-results.ann*] - a Karma annotation file,
  showing the location of detected sources. This is produced when
  **flagKarma=true**, which is the default (contrary to standard
  Duchamp behaviour)
* **ds9File** [*selavy-results.reg*] - a DS9 region file, showing the
  location of detected sources. This is produced when
  **flagDS9=true**, which is the default (contrary to standard Duchamp
  behaviour)
* **casaFile** [*selavy-results.crf*] - a CASA region file, showing
  the location of detected sources. This is produced when
  **flagCASA=true**, which is the default (contrary to standard
  Duchamp behaviour)
* **logFile** [*selavy-Logfile.txt* / selavy-Logfile-Master.txt /
  selavy-Logfile-?.txt] - the logfiles, showing lists of intermediate
  detections (before the final merging), as well as pixel-level
  details on the final detection list. The first default listed is the
  default when running serial processing. The other two come from the
  distributed-processing case. In this case, the parameter's value has
  either '-Master' or '-?' (where ? is replaced by the worker number,
  starting at 0) inserted before the suffix, or at the end if there is
  no suffix in the name provided. Only the master file (or the sole
  logfile in the serial case) has the pixel-level details of the final
  detections. These files will not be produced unless you set
  **flagLog=true**.
* **votFile** [*selavy-results.xml*] - a VOTable of the final list of
  detections. This is produced when **flagVOT=true** (the default,
  unlike standard Duchamp).
* **binaryCatalogue** [*selavy-catalogue.dpc*] - a binary format
  catalogue of detected sources that can be re-used by Selavy or
  Duchamp.

ASKAP-specific output
~~~~~~~~~~~~~~~~~~~~~

The following files are produced as a result of the new features
implemented in the ASKAP source finder:

* **subimageAnnotationFile** [*selavy-SubimageLocations.ann*] - a
  Karma annotation file showing the locations of the subimages used
  (see "Distributed Processing" section above). Lines are drawn
  showing the outer borders of each worker's subimage, and each
  subimage is labelled with the worker number (starting at 1).
* CASDA-suitable catalogues - in conjunction with the CASDA project
  (CSIRO ASKAP Science Data Archive), catalogues of islands and
  components (fitted 2D Gaussians) are written in both ASCII (.txt)
  and VOTable (.xml) formats. These take their names from the
  **resultsFile** detailed above: where that is *selavy-results.txt*,
  the islands catalogue will be *selavy-results.islands.txt* and the
  components catalogue will be *selavy-results.components.txt*, with a
  *.xml* extension for the VOTables. An example of the components
  catalogue can be found at :doc:`postprocessing`, while an example
  island catalogue follows.
* Fitting results - when Gaussian fitting is done for the continuum
  sources, several files are produced: a catalogue in ASCII & VOTable
  format (differing from the CASDA-format components catalogue), and
  annotation files showing the location of fitted components. See
  :doc:`postprocessing` for details of the content of these files.
* Images: when the variable-threshold option is used, the user can opt
  to write out relevant maps to CASA images. These include the noise
  map, detection threshold, and signal-to-noise ratio. These are
  described in more detail in :doc:`preprocessing`. Additionally, when
  the curvature-map option in the Gaussian fitting is used, the
  curvature map can be written to a CASA image - consult
  :doc:`postprocessing` for information.
* Extracted spectra and images: :doc:`extraction` describes various
  ways to extract data from the input image relating to individual
  detections. These can include integrated spectra, moment maps,
  cutout images or cubelets. All are saved to CASA-format images.
* There will also be a log file produced by Selavy that contains the
  stdout logging information - this is described in
  :doc:`../general/logging` (note the difference with the Duchamp log
  file described in the previous section).

The island catalogue will look something like the following:

.. code-block:: bash

 #         island_id    island_name n_components ra_hms_cont dec_dms_cont ra_deg_cont dec_deg_cont       freq maj_axis min_axis pos_ang    flux_int   flux_peak x_min x_max y_min y_max    n_pix   x_ave   y_ave   x_cen   y_cen x_peak y_peak flag_i1 flag_i2 flag_i3 flag_i4                                                                                             comment
 #                --                                                            [deg]        [deg]      [MHz] [arcsec] [arcsec]   [deg]       [mJy]  [mJy/beam]
      SBnull_image_1 J222645-623530            1  22:26:45.4    -62:35:30  336.689117   -62.591704      864.0     1.24     0.74  157.31     1.43677      1.2909  2350  2361   290   308      178 2355.71  299.10 2355.36  300.08   2355    300       0       0       0       0
      SBnull_image_2 J231717-613700            1  23:17:17.7    -61:37:00  349.323782   -61.616938      864.0     1.20     0.75  163.07     1.25480      1.1303   580   592   603   619      180  585.72  611.03  585.53  611.04    586    611       0       0       0       0
      SBnull_image_3 J231447-621212            1  23:14:47.5    -62:12:12  348.698019   -62.203388      864.0     1.16     0.84  151.38     0.91437      0.8587   682   696   434   451      197  688.52  442.41  688.01  442.62    688    443       0       0       0       0
      SBnull_image_4 J231034-633000            1  23:10:34.1    -63:30:00  347.642163   -63.499988      864.0     1.18     0.88  157.61     1.07729      0.8547   851   866    56    71      190  858.75   63.37  858.93   63.64    859     64       0       0       0       0

The columns used are:

* *island_id* is the unique identifier for the island. The ID string
  is made up of a scheduling block ID, the image name (the above
  example used an image called *image.fits*), followed by a unique
  identifier, which is just a numerical counter.
* *island_name* is the "IAU-format" name taken from the J2000 position
  of the island's centroid.
* *n_components* indicates how many components were fitted to
  this island.
* The position of the island is indicated by both HMS/DMS-formatted
  strings and decimal degrees for RA and DEC.
* *freq* indicates the frequency of the image.
* Estimates of the size and orientation of the island are provided
  with *maj_axis*, *min_axis* and *pos_ang* -- these are *not* fitted
  values, but calculated by the Duchamp code based on detected
  pixels.
* The integrated and peak fluxes are given by *flux_int* and
  *flux_peak*.
* The pixel ranges are shown by *x_min*, *x_max*, *y_min* and
  *y_max*, with *n_pix* giving the total number of detected pixels. 
* Three estimates of the "centre" of the island are shown: *x_ave* &
  *y_ave* give the average pixel in each axis; *x_cen* & *y_cen* give
  the centroid, or flux-weighted-average; and *x_peak* & *y_peak* give
  the location of the peak pixel.
* There are placeholders for four flags, but these are not yet used. 

Output-related parameters
~~~~~~~~~~~~~~~~~~~~~~~~~

+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|*Parameter*              |*Type*        |*Default*                   |*Description*                                                                                   |
+=========================+==============+============================+================================================================================================+
|SBid                     |string        |null                        |The Scheduling block ID. Currently this is only used for the ID strings for the islands and     |
|                         |              |                            |components.                                                                                     |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|verbose                  |bool          |false                       |Controls the verbosity for the Duchamp-specific code. **verbose=true** means more information   |
|                         |              |                            |about the Duchamp functions                                                                     |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|pixelCentre              |string        |centroid                    |How the central pixel value is defined in the output catalogues (can take values of 'centroid', |
|                         |              |                            |'average' or 'peak').                                                                           |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|resultsFile              |string        |selavy-results.txt          |The text file holding the catalogue of results. Can also use **outFile** for compatbility with  |
|                         |              |                            |Duchamp.                                                                                        |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagSeparateHeader       |bool          |false                       |Whether the "header" containing the summary of input parameters should be written to a separate |
|                         |              |                            |file from the table of results. If produced, it will be called selavy-results.hdr.              |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|subimageAnnotationFile   |string        |""                          |The filename of a Karma annotation file that is created to show the boundaries of the subimages |
|                         |              |                            |(see description below). If empty, no such file is created.                                     |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagLog                  |bool          |false                       |Produce a Duchamp-style log file, recording intermediate detections (see above). The workers    |
|                         |              |                            |will produce selavy-Logfile.%w.txt, (where %w is the worker number, in the usual fashion) and   |
|                         |              |                            |the master will produce selavy-Logfile.Master.txt.                                              |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagVOT                  |bool          |true                        |Produce a VOTable of the results.                                                               |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|votFile                  |string        |selavy-results.txt          |The VOTable containing the catalogue of detections.                                             |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagWriteBinaryCatalogue |bool          |true                        |Produce a binary catalogue compatible with Duchamp (that can be loaded into Duchamp along with  |
|                         |              |                            |the image to produce plots of the detections).                                                  |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|binaryCatalogue          |string        |selavy-catalogue.dpc        |The binary catalogue.                                                                           |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagTextSpectra          |bool          |false                       |Produce a file with text-based values of the spectra of each detection.                         |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|spectraTextFile          |string        |selavy-spectra.txt          |The file containing ascii spectra of each detection.                                            |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|objectList               |string        |*no default*                |A comma-separated list of objects that will be used for the post-processing. This is inherited  |
|                         |              |                            |from Duchamp, where it can be used to only plot a selection of sources. This is most useful for |
|                         |              |                            |re-running with a previously-obtained catalogue.  In Selavy, this will only be applied to the   |
|                         |              |                            |spectraTextFile and spectral extraction options (see the :doc:`extraction` page for details on  |
|                         |              |                            |the latter).                                                                                    |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagKarma                |bool          |true                        |Produce a Karma annotation plot. *Note the different default from standard Duchamp.*            |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|karmaFile                |string        |selavy-results.ann          |The Karma annoation file showing the location of detected objects.                              |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagDS9                  |bool          |true                        |Produce a DS9 region file.  *Note the different default from standard Duchamp.*                 |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|ds9File                  |string        |selavy-results.reg          |The DS9 region file showing the location of detected objects.                                   |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|flagCasa                 |bool          |true                        |Produce a CASA region file.  *Note the different default from standard Duchamp.*                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|casaFile                 |string        |selavy-results.crf          |The CASA region format file showing the location of detected objects.                           |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|drawBorders              |bool          |true                        |Whether to draw the object borders in the annotation file. If false, only draw a circle with    |
|                         |              |                            |radius proportional to the object's size.                                                       |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|fitResultsFile           |string        |selavy-fitResults.txt       |The ASCII file containing the results of the Guassian fitting                                   |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|fitAnnotationFile        |string        |selavy-fitResults.ann       |A Karma annotation file showing the location, size & shape of fitted components.                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|fitBoxAnnotationFile     |string        |selavy-fitResults.boxes.ann |A Karma annotation file showing the location and size of boxes used in the Gaussian fitting     |
|                         |              |                            |(only produced if Fitter.fitJustDetection = false).                                             |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|precFlux                 |int           |3                           |Precision for the flux values in the output files                                               |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|precVel                  |int           |3                           |Precision for the velocity values in the output files                                           |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|precSNR                  |int           |2                           |Precision for the SNR values in the output files                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+
|sortingParam             |string        |vel                         |The parameter with which to sort the output list: x-value, y-value, z-value, ra, dec, vel, w50, |
|                         |              |                            |iflux, pflux, snr. A - prepended to the parameter reverses the order of the sort.               |
|                         |              |                            |                                                                                                |
+-------------------------+--------------+----------------------------+------------------------------------------------------------------------------------------------+

.. _`Extraction`: extraction.html
