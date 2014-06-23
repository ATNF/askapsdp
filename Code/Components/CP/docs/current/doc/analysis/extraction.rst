Extraction of Spectra, Images and Cubelets
==========================================

This page describes the way spectral and image data products can be extracted for each detected source. Using these tools, the user can get integrated spectra, noise spectra, moment maps, and cube cutouts for each (or a selection) of the detected sources.


Spectral Extraction
-------------------

This set of parameters allows, for each detected source, the extraction of spectra at the corresponding position in a spectral cube. Typical use cases include:

* Extracting full-Stokes spectra from a continuum cube for a source detected in a taylor.0 MFS image.
* Extracting an integrated or peak spectrum of a spectral-line source (eg. an HI detection). In this case the spectral cube is the same dataset as that used for the source detection.

*Note*: At this point the pixel coordinates are used to get the location in the spectral cube, so the detection image and the spectral cube should have the same WCS. This will be updated as needed in future releases.

The initial use case follows the specification provided by POSSUM. The user provides a spectral cube, a list of polarisations to be extracted, and a box width. The spectrum is summed in each channel within this box, centred on the peak (or centroid or average) position of the detected object. This spectrum can optionally be scaled by the response of the beam over the same box, such that the resulting spectrum gives the flux of a point source at that location. The beam used defaults to the beam from the image header, although a 'beam log' (produced by the `makecube`_ utility) can be provided so that each channel is scaled by the appropriate restoring beam. The beam log should have columns: index | image name | major axis [arcsec] | minor axis [arcsec] | position angle [deg]. 

**We have changed the way spectral-line data is produced, so makecube may not be used. This area still requires some work, but one could construct the beam log following this format.**

Here is an example of the start of a beam log:

::

  #Channel Image_name BMAJ[arcsec] BMIN[arcsec] BPA[deg]
  0 image.i.cube.clean_ch0.restored 65.7623 36.4139 -54.7139
  1 image.i.cube.clean_ch1.restored 65.7981 36.4222 -54.7605
  2 image.i.cube.clean_ch2.restored 65.8365 36.4324 -54.7985
  3 image.i.cube.clean_ch3.restored 65.8733 36.4413 -54.845
  4 image.i.cube.clean_ch4.restored 65.9082 36.4512 -54.8919

At present, the decision to scale by the beam is made at the parset input stage, but future versions of the algorithm could make this a dynamic choice based on the source in question.

The user can select which polarisations / Stokes parameters should be included in the output spectra. There are three options here for the case of multiple polarisations, with **polarisation="IQUV"**

* The user provides a single spectral cube that has all polarisations listed: **spectralCube=image.iquv.cube**
* The user provides a list of spectral cubes that have a 1-1 match with the list of polarisations provided. The list is provided as a comma-separated list enclosed in square brackets, and the order of images should match the order of polarisations: **spectralCube=[image.i.cube,image.q.cube,image.u.cube,image.v.cube]** 
* The user provides a single filename that uses a "%p" wildcard in place of the polarisation name (in lower case): **spectralCube=image.%p.cube**

The second use case is triggered by setting **extractSpectra.useDetectedPixels=true**. This results in the spectrum being summed over all spatial pixels in which the object in question was detected. If **extractSpectra.scaleSpectraByBeam=true**, then the spectrum is scaled by the area of the beam (in the same way the integrated flux of a Duchamp detection is scaled by the beam). 

*IMPORTANT NOTE* - the spectra are written to *CASA images*, rather than FITS files.

Additionally, Selavy inherits Duchamp's ability to save the spectra to an ASCII text file. This is controlled by the parameters **flagTextSpectra** and **spectraTextFile**.

You can select particular objects for spectral extraction, either to CASA images or ASCII text, by using the **objectList** parameter and providing a comma-separated list of object IDs.

.. _makecube: ../cp_utils/makecube.html


Parameters for spectral extraction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|*Parameter*                       |*Type*          |*Default*          |*Explanation*                                                                                       |
+==================================+================+===================+====================================================================================================+
|extractSpectra                    |bool            |false              |Whether to extract spectra for each detected source                                                 |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.spectralCube       |vector<string>  |[]                 |A set of input spectral cubes from which the spectra will be extracted, given as a comma-separated  |
|                                  |                |                   |list within square brackets. If just one cube is required, the square brackets are optional.        |
|                                  |                |                   |                                                                                                    |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.spectralOutputBase |string          |""                 |The base used for the output spectra. The filename for a given source will be this string appended  |
|                                  |                |                   |with "_ID", where ID is the source's ID number from the results list.                               |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.polarisation       |vector<string>  |I                  |The set of Stokes parameters for which spectra should be extracted.  Multiple polarisations can be  |
|                                  |                |                   |provided in a number of ways: "IQUV", ["I","Q","U","V"] etc. See text for requirements.             |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.spectralBoxWidth   |int             |5                  |The width of the box to be applied in the extraction.                                               |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.pixelCentre        |string          |peak               |Which object location to use. Can be "peak", "centroid", or "average", in the same way as the       |
|                                  |                |                   |standard Duchamp input parameter pixelCentre.                                                       |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.useDetectedPixels  |bool            |false              |Whether to use all detected pixels of the object, and integrating over them, rather than applying a |
|                                  |                |                   |box.                                                                                                |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.scaleSpectraByBeam |bool            |true               |Whether to scale the resulting spectra by the beam extracted over the same box, or, in the case of  |
|                                  |                |                   |useDetectedPixels=true, by the area of the beam (using the same correction as in Duchamp).          |
|                                  |                |                   |                                                                                                    |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|extractSpectra.beamLog            |string          |""                 |The name of a 'beam log' produced by `makecube`_. If provided (and if the scaleSpectraByBeam flag is|
|                                  |                |                   |set), each channel is independently corrected by the relevant restoring beam. If not provided, the  |
|                                  |                |                   |beam from the image header is used instead.                                                         |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|flagTextSpectra                   |bool            |false              |Produce a file with text-based values of the spectra of each detection.                             |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|spectraTextFile                   |string          |selavy-spectra.txt |The file containing ascii spectra of each detection.                                                |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+
|objectList                        |string          |*no default*       |A comma-separated list of objects that will be used for the post-processing. This is inherited from |
|                                  |                |                   |Duchamp, where it can be used to only plot a selection of sources. This is most useful for          |
|                                  |                |                   |re-running with a previously-obtained catalogue.  In Selavy, this will only be applied to the       |
|                                  |                |                   |spectraTextFile and spectral extraction options. If not provided, all objects will be processed.    |
|                                  |                |                   |                                                                                                    |
+----------------------------------+----------------+-------------------+----------------------------------------------------------------------------------------------------+


Noise spectra
-------------

The same algorithms can be applied to extract noise spectra for each object. In this case, the box used is defined by a multiple of beam areas (defaulting to 50, as per the POSSUM specification). The box is taken to be a square box with the same area as requested. For each channel, the noise rms level is measured within that box to produce the noise spectrum.

As for the source spectrum, a polarisation can be indicated as the Stokes parameter from which to measure the noise. Only one Stokes parameter is used - if more than one is provided, only the first is used. The same rules for accessing the spectral cube are applied as described above.

The **objectList** parameter applies to the noise spectra as well.

Parameters for noise spectra extraction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------+---------------+------------+---------------------------------------------------+
|*Parameter*                            |*Type*         |*Default*   |*Explanation*                                      |
+=======================================+===============+============+===================================================+
|extractNoiseSpectra                    |bool           |false       |Whether to extract a noise spectrum from around    |
|                                       |               |            |each detected source                               |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.spectralCube       |vector<string> |[]          |As above. If more than one cube is given, only the |
|                                       |               |            |first is used.                                     |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.spectralOutputBase |string         |""          |As above.                                          |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.polarisation       |vector<string> |I           |As above. If more than one is provided, only the   |
|                                       |               |            |first is used.                                     |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.noiseArea          |float          |50.0        |The number of beam areas over which to measure the |
|                                       |               |            |noise.                                             |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.robust             |bool           |true        |Whether to use robust methods to estimate the      |
|                                       |               |            |noise.                                             |
+---------------------------------------+---------------+------------+---------------------------------------------------+


Moment-map extraction
---------------------

Similar facilities exist for creating and extracting moment maps for spectral-line detections. This is capable of creating the total intensity (moment-0) map, the intensity-weighted mean velocity field (moment-1 map) and the intensity-weighted velocity dispersion (moment-2 map). The default behaviour is to produce all three, although one may use the **moments** parameter to select individual maps (e.g. **moments=[0,1]** to select just the total intensity and mean velocity field maps).

There is one key choice to be made that affects the appearance of these maps, and that is what voxels to include in the calculations. By setting **useDetectedPixels=true**, the only pixels included in the calculations will be those that actually form part of the detected object. Pixels that do not form part of the object are masked in the final images. If **useDetectedPixels=false**, then the moment maps will be made with all pixels within the channel range of the detected object, whether or now they formed part of that object. 

The spatial size of the maps is determined in one of two ways. If **spatialMethod=box**, then the spatial size is at least the size of the detected object, padded out on each side by a given number of pixels if desired (by using the **padSize** parameter). If **spatialMethod=fullfield**, then the full spatial size of the input cube is used.

The output filenames can be specified using a special wildcard: '%m' will be replaced with the moment number, so that if one provides **momentOutputBase=myImage_mom%m**, then the first object's moment-0 map will go into myImage_mom0_1 and its moment-1 map will go to myImage_mom1_1. As above, the object ID is appended to the base name in the form "_ID".

As above, the output images are created in *CASA format*.
 

Parameters for moment-map extraction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|*Parameter*                            |*Type*         |*Default*   |*Explanation*                                                   |
+=======================================+===============+============+================================================================+
|extractMomentMap                       |bool           |false       |Whether to extract moment maps.                                 |       
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractMomentMap.spectralCube          |vector<string> |[]          |As above. If more than one cube is given, only the first is     |       
|                                       |               |            |used.                                                           |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractMomentMap.momentOutputBase      |string         |""          |Base name for the moment maps. If more than one moment is being |       
|                                       |               |            |used, use '%m' to represent the moment number.  The name is     |
|                                       |               |            |appended with "_ID", where ID is the object ID number.          |       
+---------------------------------------+---------------+------------+----------------------------------------------------------------+       
|extractMomentMap.moments               |vector<int>    |[0]         |Which moment maps to create.                                    |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractMomentMap.spatialMethod         |string         |box         |Either "box" (cutout is restricted to the immediate vicinity of |
|                                       |               |            |the detection, padded by **padSize**), or "fullfield" (the      |
|                                       |               |            |entire spatial size of the input cube).                         |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractMomentMap.padSize               |int            |5           |When using **spatialMethod=box**, a border of this many pixels  |
|                                       |               |            |is added to the edges of the image, surrounding the spatial     |
|                                       |               |            |extent of the detection.                                        |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractMomentMap.useDetectedPixels     |bool           |true        |Whether to just use the detected pixels in calculating the      |
|                                       |               |            |moment maps (**true**) or to use all pixels within the detected |
|                                       |               |            |object's spectral range.                                        |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+


Cubelet extraction
------------------

The final form of data product extraction is to extract 'cubelets' - cutout cubes surrounding the detected object. These have no processing applied to them other than the trimming, and so provide a way of looking at the data directly relevant to the detected object without having to load the entire input image cube.

The cubelet size is taken from the outer dimensions of the detected object, and can be padded by a certain number of pixels in the spatial and spectral directions. To specify the padding amount, use the **padSize** parameter, giving a vector with two elements. The first is the pad size used in the spatial direction, the second is for the spectral direction. If only one value is given it is applied to both directions.

The input data need not be a cube, of course - it is possible to run this on a continuum image and it will work in the same way.

As above, the output cubes are created in *CASA format*.
 
Parameters for cubelet extraction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|*Parameter*                            |*Type*         |*Default*   |*Explanation*                                                   |
+=======================================+===============+============+================================================================+
|extractCubelet                         |bool           |false       |Whether to extract cubelets.                                    |       
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractCubelet.spectralCube            |vector<string> |[]          |As above. If more than one cube is given, only the first is     |       
|                                       |               |            |used.                                                           |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
|extractCubelet.cubeletOutputBase       |string         |""          |Base name for the cubelet files.                                |       
+---------------------------------------+---------------+------------+----------------------------------------------------------------+       
|extractCubelet.padSize                 |vector<int>    |[5,5]       |Number of pixels to add to the edge of the detection in the     |
|                                       |               |            |spatial and spectral directions respectively. If a single       |
|                                       |               |            |integer is provided, this is applied to both spatial and        |
|                                       |               |            |spectral directions.                                            |
+---------------------------------------+---------------+------------+----------------------------------------------------------------+
