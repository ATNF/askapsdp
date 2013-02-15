Extraction of Spectra
=====================


Spectral Extraction
-------------------

This set of parameters allows, for each detected source, the extraction of spectra at the corresponding position in a spectral cube. Typical use cases include:

* Extracting full-Stokes spectra from a continuum cube for a source detected in a taylor.0 MFS image.
* Extracting an integrated or peak spectrum of a spectral-line source (eg. an HI detection). In this case the spectral cube is the same dataset as that used for the source detection.

*Note* At this point the pixel coordinates are used to get the location in the spectral cube, so the detection image and the spectral cube should have the same WCS. This will be updated as needed in future releases.

The initial use case follows the specification provided by POSSUM. The user provides a spectral cube, a list of polarisations to be extracted, and a box width. The spectrum is summed in each channel within this box, centred on the peak (or centroid or average) position of the detected object. This spectrum can optionally be scaled by the response of the beam over the same box, such that the resulting spectrum gives the flux of a point source at that location. At present, the decision to scale by the beam is made at the parset input stage, but future versions of the algorithm could make this a dynamic choice based on the source in question.

The user can select which polarisations / Stokes parameters should be included in the output spectra. There are three options here for the case of multiple polarisations, with **polarisation="IQUV"**

* The user provides a single spectral cube that has all polarisations listed: **spectralCube=image.iquv.cube**
* The user provides a list of spectral cubes that have a 1-1 match with the list of polarisations provided. The list is provided as a comma-separated list enclosed in square brackets, and the order of images should match the order of polarisations: **spectralCube=[image.i.cube,image.q.cube,image.u.cube,image.v.cube]** 
* The user provides a single filename that uses a "%p" wildcard in place of the polarisation name (in lower case): **spectralCube=image.%p.cube**

The second use case is triggered by setting extractSpectra.useDetectedPixels=true. This results in the spectrum being summed over all spatial pixels in which the object in question was detected. If extractSpectra.scaleSpectraByBeam=true, then the spectrum is scaled by the area of the beam (in the same way the integrated flux of a Duchamp detection is scaled by the beam). 

*IMPORTANT NOTE* - the spectra are written to *CASA images*, rather than FITS files.

Parameters for spectral extraction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|*Parameter*                       |*Type*          |*Default*   |*Explanation*                                                                    |
+==================================+================+============+=================================================================================+
|extractSpectra                    |bool            |false       |Whether to extract spectra for each detected source                              |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.spectralCube       |vector<string>  |[]          |A set of input spectral cubes from which the spectra will be extracted, given as |
|                                  |                |            |a comma-separated list within square brackets. If just one cube is required, the |
|                                  |                |            |square brackets are optional.                                                    |
|                                  |                |            |                                                                                 |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.spectralOutputBase |string          |""          |The base used for the output spectra. The filename for a given source will be    |
|                                  |                |            |this string appended with "_ID", where ID is the source's ID number from the     |
|                                  |                |            |results list. When run through the Selavy service, the filename will have        |
|                                  |                |            |"selavy-SPECTRA-" prepended to it.                                               |
|                                  |                |            |                                                                                 |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.polarisation       |vector<string>  |I           |The set of Stokes parameters for which spectra should be extracted.  Multiple    |
|                                  |                |            |polarisations can be provided in a number of ways: "IQUV", ["I","Q","U","V"]     |
|                                  |                |            |etc. See text for requirements.                                                  |
|                                  |                |            |                                                                                 |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.spectralBoxWidth   |int             |5           |The width of the box to be applied in the extraction.                            |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.pixelCentre        |string          |peak        |Which object location to use. Can be "peak", "centroid", or "average", in the    |
|                                  |                |            |same way as the standard Duchamp input parameter pixelCentre.                    |
|                                  |                |            |                                                                                 |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.useDetectedPixels  |bool            |false       |Whether to use all detected pixels of the object, and integrating over them,     |
|                                  |                |            |rather than applying a box.                                                      |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+
|extractSpectra.scaleSpectraByBeam |bool            |true        |Whether to scale the resulting spectra by the beam extracted over the same box,  |
|                                  |                |            |or, in the case of useDetectedPixels=true, by the area of the beam (using the    |
|                                  |                |            |same correction as in Duchamp).                                                  |
|                                  |                |            |                                                                                 |
+----------------------------------+----------------+------------+---------------------------------------------------------------------------------+

Noise spectra
-------------

The same algorithms can be applied to extract noise spectra for each object. In this case, the box used is defined by a multiple of beam areas (defaulting to 50, as per the POSSUM specification). The box is taken to be a square box with the same area as requested. For each channel, the noise rms level is measured within that box to produce the noise spectrum.

As for the source spectrum, a polarisation can be indicated as the Stokes parameter from which to measure the noise. Only one Stokes parameter is used - if more than one is provided, only the first is used. The same rules for accessing the spectral cube are applied as described above.

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
|extractNoiseSpectra.spectralOutputBase |string         |""          |As above. When run through the Selavy service, the |
|                                       |               |            |filename will have "selavy-NOISE-SPECTRA-"         |
|                                       |               |            |prepended to it.                                   |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.polarisation       |vector<string> |I           |As above. If more than one is provided, only the   |
|                                       |               |            |first is used.                                     |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.noiseArea          |float          |50.         |The number of beam areas over which to measure the |
|                                       |               |            |noise.                                             |
+---------------------------------------+---------------+------------+---------------------------------------------------+
|extractNoiseSpectra.robust             |bool           |true        |Whether to use robust methods to estimate the      |
|                                       |               |            |noise.                                             |
+---------------------------------------+---------------+------------+---------------------------------------------------+
