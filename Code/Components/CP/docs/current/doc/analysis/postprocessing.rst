Post-processing of detections
=============================

Once the list of detected objects has been obtained, there are several post-processing steps that can be utilised. We describe here mask optimisation, two-dimensional component fitting, and spectral index & curvature measurement.


Mask optimisation
-----------------

The WALLABY team have provided an algorithm to optimise the source mask (equivalent to the Duchamp concept of growing detections), such that the integrated flux of the detection is maximised subject to the noise present. This has been provided to avoid issues with the fact that the Duchamp-based parameterisation makes use only of the detected pixels, so structure below the detection threshold is not taken into account. Growing the object in this way increases the number of faint pixels contributing to source parameterisation.
 
Unlike the Duchamp-derived "growing" algorithm (**Selavy.flagGrowth=true**), the growing process does not use a flux or signal-to-noise threshold. Rather, it adds elliptical annuli to the source until the total flux of the *annulus* is negative or a specified maximum number of iterations is reached. It does this for all channels within ±W50 of the central spectral channel.

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

To aid evaluation of this algorithm, the mask FITS output of Duchamp has been enabled in Selavy, although *only for the serial case*. If running a multiple-node job, the mask will not be written. When written, the mask will be called *selavy-MASK-IMG.fits*, where the IMG refers to the input image. 

 .. _`Selavy page`: https://pm.atnf.csiro.au/askap/projects/sup/wiki/Wiki_sup_wg_2_sourcefinding_service

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


Source fitting
--------------

For continuum images, we have implemented the ability to fit 2D Gaussian components to detected sources. Note that no such facility yet exists for 3D cubes. For the purposes of this section, we adopt the terminology used elsewhere of describing the detected objects (that is, collections of pixels above the detection threshold) as 'islands' which have 'components' fitted to them. For Selavy, the islands are the result of running the detection algorithms, and it is the islands that are written to the catalogue given by **Selavy.outFile** (defaulting to *selavy-results.txt*).

Setup
~~~~~

The fitting itself is done by the *fitGaussian* function from the *casacore* package. This returns a set of parameters for each Gaussian: peak flux, x-pixel centre, y-pixel centre, major axis FWHM, axial ratio, position angle.

The pixels that are used in the fit are chosen in one of two ways. If **Selavy.Fitter.fitJustDetection=true**, then only the island pixels (ie. those that were detected) are used in the fit - no "background" pixels are used. This avoid confusion with possible neighbouring sources.

Alternatively, if **Selavy.Fitter.fitJustDetection=false**, the fitting is done using all pixels in a box surrounding the island. This box is defined by padding a border of a minimum number of pixels (defined by the **boxPadSize** parameter) in all directions around the island pixels (moving out from the extremities of the detected pixels). Only the pixels lying within this box are used in the fit. This approach is more consistent with the approach used in the FIRST survey.

Note that all parameters governing the fitting require a **Selavy.Fitter.** prefix, but for convenience this is left out in the text that follows.

Noise determination
~~~~~~~~~~~~~~~~~~~

The noise level (ie. the standard deviation of the noise background) is used by the fitting function to weight the fitting function and obtain the chi-squared value. This noise level is not that used by the detection algorithm, but rather one obtained by finding the median absolute deviation from the median (MADFM) in a square box centred on the island's peak pixel. The side length of the box is governed by the **noiseBoxSize** parameter, which defaults to 101 pixels. Note that it is different to the box mentioned above. The MADFM is converted to an equivalent rms for a Gaussian noise distribution (by dividing by 0.674888). 

It is possible to do the fit without calculating the noise level, by seeting **useNoise = false**. This sets the pixel sigma value to 1 for each pixel, effectively removing the noise from the chi-squared calculation.



Initial estimation of parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The *fitGaussian* function requires an initial estimate of the parameters. There are two ways Selavy obtains this initial estimate. The first way (the default) uses the following recursive algorithm to provide a list of sub-components (being distinct peaks within an island):

* Using the island pixels, define a set of parameters: peak flux and location are obvious, while the major & minor axes & position angle are obtained from Duchamp algorithms, using a separate detection process at half the peak flux (to get the full width at half maximum).
* Define a set of sub-thresholds (the number of these is given by the **numSubThresholds** parameter) spaced either linearly or logarithmically between the peak flux and the detection threshold (the separation between sub-thresholds is constant in either log or linear space depending on whether the input parameter **logarithmicThresholds** is true or false). 
* For each threshold, search the box surrounding the object, and record the number of separate detections.

 - If there is more than one separate object, call the getSubComponentList function on each of these and add the result to the vector list.

* When you reach the final threshold, add the initial set of parameters to the vector list and return the vector list.

If the Gaussian fitting fails to provide a good fit, these initial estimates can be returned as the results, with a flag indicating they are only estimates. Whether this is done is governed by the parameter **useGuessIfBad**, which defaults to **true**. If an estimate is reported in the results output, the final column *Guess?* will take the value 1, else it will be 0.

The second approach attempts to replicate the algorithm described in `Hancock et al. (2012), MNRAS 422, 1812`_ (which is used in Paul's 'Aegean' source-finder). This creates a curvature map using a discrete 2D Laplacian kernel. The curvature map, in combination with the mask of detected pixels, is used to locate local maxima within the island. The image is convolved with the 3x3 kernel - note that this means the single row of pixels around the edge will not have a curvature value assigned.

 .. _Hancock et al. (2012), MNRAS 422, 1812: http://adsabs.harvard.edu/abs/2012MNRAS.422.1812H
 
A point source is placed at each local maxima to provide the initial estimate of components. To use this mode, set **useCurvature=true**. The curvature map can be saved to a CASA image by setting the **curvatureImage** parameter.



Fitting
~~~~~~~

If **numGaussFromGuess=true** (the default), the number of Gaussians that are fitted to the island is exactly equal to the number of subcomponents in the initial estimate. If, however, **numGaussFromGuess=false**, then the number of Gaussians fitted is between 1 and **maxNumGauss**. The subcomponents are chosen as initial estimates in order of their peak flux. If there are more Gaussians needed than there are subcomponents, we simply cycle through the list.

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

If **numGaussFromGuess=true**, then we only use the same number of Gaussians as the number of components in the initial estimate. If this is **false**, the the behaviour is governed by the **stopAfterFirstGoodFit** parameter. If **stopAfterFirstGoodFit=true**, once the first acceptable fit is found  (starting with a single Gaussian), the fitting is stopped. Multiple Gaussians are fitted only if fewer Gaussians do not give an acceptable fit. If **stopAfterFirstGoodFit=false** then the fitting using one through to the maximum number of Gaussians, and the best fit is chosen to be the one that passes all the above criteria and has the lowest reduced chi-squared value.

.. _Becker, White & Helfand 1995: http://adsabs.harvard.edu/abs/1995ApJ...450..559B

A note on the reduced chi-squared
.................................

The expression used to calculate the reduced chi-squared as shown above is fine if the pixels are independent. However, this is not the case for radio data, where neighbouring pixels are correlated due to the finite beam size. It is not immediately obvious what the correct way to estimate the reduced chi-squared is. It may be that, formally, a different metric should be used in assessing the goodness-of-fit (since an underlying assumption of the chi-squared test is that the pixels are independent).

Note that, leaving aside the formal requirements of the statistical test, this is primarily a problem when comparing different successful fits that have different numbers of Gaussians. The determination of the best fit for a given number of Gaussians should not be affected (although the second of our acceptance criteria might have to change).

Output files
............

Several files are produced to show the results of the Gaussian fitting. The first, named by **Selavy.fitResultsFile** (defaulting to selavy-fitResults.txt), shows the final set of results from the Gaussian fitting. An example header plus a first line for the file is as follows below. 

.. code-block:: bash

 #      ID           Name         RA        DEC      X      Y       F_int     F_peak  F_int(fit)  F_pk(fit)  Maj(fit)  Min(fit) P.A.(fit) Maj(fit_deconv.)   Min(fit_deconv.) P.A.(fit_deconv.)      Alpha      Beta  Chisq(fit)  RMS(image)  RMS(fit) Nfree(fit) NDoF(fit) NPix(fit) NPix(obj) Guess?
 #      --             --      [deg]      [deg]  [pix]  [pix]        [Jy]  [Jy/beam]        [Jy]  [Jy/beam]  [arcsec]  [arcsec]     [deg]         [arcsec]           [arcsec]             [deg]         --        --          --   [Jy/beam] [Jy/beam]         --        --        --        --     --
        1a J121529-464244 183.871792 -46.711242 1919.0  388.0     0.15264     0.1291      0.1291     0.1291    58.661    43.973    147.38            0.012            0.00055            294.77  -5.865861 -20.13710    999.0000     0.00516     0.000          0         0         0        78      1
        1b J121529-464244 183.880826 -46.697610 1917.0  393.0     0.15264     0.1291      0.0218     0.0218    58.661    43.973    147.38            0.012            0.00055            294.77   1.223772  -0.13692    999.0000     0.00516     0.000          0         0         0        78      1
        2a J121557-450037 183.987820 -45.010316 1917.3 1000.9     0.13569     0.1136      0.1566     0.1171    68.806    50.160    166.85           40.537           15.24053            318.09  -3.117519  -6.41822     30.9322     0.00290     0.622          6        73        80        80      0

To summarise the columns:

* *ID* is a unique ID for the component. It comprises the ID number of the island, plus a letter indicating the order of components for that island.
* *Name* is the name taken from the island.
* *RA*, *Dec*, *X* and *Y* are the world and pixel locations of the *component*.
* *F_int* and *F_peak* are values for the island as calculated by the Duchamp code, and reported in the Duchamp results file given by **Selavy.outFile**. These are the same for each comonent of that island.
* *F_int(fit)* and *F_pk(fit)* are the integrated & peak fluxes from the fitted Gaussians. 
* Alpha and Beta are the spectral index and spectral curvature terms. These are only provided when the appropriate flags are set - see `Spectral Terms`_. 
* *Maj*, *Min* and *P.A.* are the major and minor FWHMs and the position angle of the fitted Gaussian, quoted for both the fit and the fit deconvolved by the beam. 
* The goodness of fit is indicated by the *Chisq(fit)* and *RMS(fit)* values, while *RMS(image)* gives the local noise surrounding the object. 
* *Nfree(fit)* is the number of free parameters in the fit, and *NDoF(fit)* is the number of degrees of freedom.
* *Npix(fit)* is the number of pixels used in doing the fit, and *Npix(obj)* is the number of pixels in the object itself (ie. detected pixels).
* A value of 1 in the *Guess?* column indicates that the "fitted" parameters come from the initial estimate (the fitting procedure failed for some reason).

If no fit was made (see components 1a and 1b in the example above), the Gaussian parameters are taken from the initial estimate, while those parameters relating to the quality of the fit are set to zero (for RMS, Nfree etc) or 999 (chisq).

A VOTable version of the fit results is also produced, with a .xml suffix. This is always produced whenever the fit results file is produced.

Two types of annotation files will also be produced:

* **fitAnnotationFile** [selavy-fitResults.ann] - a Karma annotation file showing the fitting results (each Gaussian component is indicated by an ellipse given by the major & minor axes and position angle of the component).
* **fitBoxAnnotationFile** [selavy-fitResults.boxes.ann] - a Karma annotation file showing the boxes used for the Gaussian fitting (if boxes were not used, ie. **fitJustDetection=true**, this file is not created). 

As well as Karma annotation files, CASA and DS9 region files will be created, with .crf and .reg suffixes respectively.

.. _Spectral Terms: postprocessing.html#spectral-index-curvature

Parameters for fitting
......................

+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|*Parameter*                          |*Type*         |*Default*                   |*Description*                                                                       |
+=====================================+===============+============================+====================================================================================+
|**Basic control parameters**         |               |                            |                                                                                    |
|                                     |               |                            |                                                                                    |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.distribFit                    |bool           |true                        |If true, the edge sources are distributed by the master node to the workers for     |
|                                     |               |                            |fitting. If false, the master node does all the fitting.                            |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.doFit                  |bool           |false                       |Whether to fit Gaussian components to the detections                                |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.fitJustDetection       |bool           |true                        |Whether to use just the detected pixels in finding the fit. If false, a rectangular |
|                                     |               |                            |box is used.                                                                        |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.fitTypes               |vector<string> |[full,psf]                  |A vector of labels for the types of fit to be done. The input format needs to be *a |
|                                     |               |                            |comma-separated list enclosed by square brackets* (as in the default). There are two|
|                                     |               |                            |default options: "full", where all 6 parameters in the Gaussian are fitted, and     |
|                                     |               |                            |"psf", where the major & minor axes and the position angle are kept fixed to the    |
|                                     |               |                            |beam size. There is also a third option, "shape", where the location and shape are  |
|                                     |               |                            |fitted, but the height of the Gaussian is kept at the object's peak flux value. The |
|                                     |               |                            |"shape" option needs to be specifically requested.                                  |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.maxNumGauss            |int            |4                           |The maximum number of Gaussians to fit to a single detection. Ignored if            |
|                                     |               |                            |**numGaussFromGuess=true**.                                                         |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.boxPadSize             |int            |3                           |When **fitJustDetection=false**, a border of at least this size is added around the |
|                                     |               |                            |detection to create a rectangular box in which the fitting is done.                 |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.stopAfterFirstGoodFit  |bool           |true                        |Whether to stop the fitting when an acceptable fit is found, without considering    |
|                                     |               |                            |fits with more Gaussian components. Ignored if **numGaussFromGuess=true**.          |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|**Initial estimates**                |               |                            |                                                                                    |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.numGaussFromGuess      |bool           |true                        |Whether the number of Gaussians fitted should be the same as the number of          |
|                                     |               |                            |components in the initial estimate (the "guess"). If false, the maximum number is   |
|                                     |               |                            |taken from **maxNumGauss**.                                                         |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.numSubThresholds       |int            |20                          |The number of levels between the detection threshold and the peak that is used to   |
|                                     |               |                            |search for subcomponents.                                                           |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.logarithmicThresholds  |bool           |true                        |Whether the sub-thresholds should be evenly spaced in log-space (true) or           |
|                                     |               |                            |linear-space (false)                                                                |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.useCurvature           |bool           |false                       |Whether to find the initial component estimates from the curvature map instead of   |
|                                     |               |                            |the sub-thresholding.                                                               |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.curvatureImage         |string         |<No default>                |The name of the CASA image in which to write the curvature map. It will be made the |
|                                     |               |                            |same size as the input image.                                                       |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.useGuessIfBad          |bool           |true                        |Whether to print the initial estimates in the case that the fitting fails           |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|**Quality control parameters**       |               |                            |                                                                                    |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.maxReducedChisq        |float          |5.0                         |The maximum value for the reduced chi-squared for a fit to be acceptable.           |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.chisqConfidence        |float          |-1.0                        |A probability value, between 0 and 1, used as a confidence level for accepting the  |
|                                     |               |                            |chi-squared value. If outside this range of values (as is the default), the test is |
|                                     |               |                            |done with the reduced chi-squared value, using the **maxReducedChisq** parameter.   |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.maxRMS                 |float          |1.0                         |The value that is passed to the FitGaussian::fit() function.                        |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.useNoise               |bool           |true                        |Whether to measure the noise in a box surrounding the island and use that as the    |
|                                     |               |                            |sigma value for each point in the fit. Setting to false has the effect of setting   |
|                                     |               |                            |the sigma to one for each point.                                                    |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.noiseBoxSize           |int            |101                         |The side length of a box centred on the peak pixel that is used to estimate the     |
|                                     |               |                            |noise level (ie. the rms) for a source: this is used for the fitting.               |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.minFitSize             |int            |3                           |The minimum number of detected pixels that an island has for it to be fit.          |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.maxIter                |int            |1024                        |The maximum number of iterations in the fit.                                        |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.maxRetries             |int            |0                           |The maximum number of retries used by the fitting routine (ie. the maxRetries       |
|                                     |               |                            |parameter for casa::FitGaussian::fit()).                                            |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.Fitter.criterium              |double         |0.0001                      |The convergence criterium for casa::FitGaussian::fit() (this does not seem to be    |
|                                     |               |                            |used in the fitting).                                                               |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|**Output files**                     |               |                            |                                                                                    |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.fitResultsFile                |string         |selavy-fitResults.txt       |The ASCII file containing the results of the Guassian fitting                       |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.fitAnnotationFile             |string         |selavy-fitResults.ann       |A Karma annotation file showing the location, size & shape of fitted components.    |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+
|Selavy.fitBoxAnnotationFile          |string         |selavy-fitResults.boxes.ann |A Karma annoation file showing the location and size of boxes used in the Gaussian  |
|                                     |               |                            |fitting (only produced if Fitter.fitJustDetection = false).                         |
+-------------------------------------+---------------+----------------------------+------------------------------------------------------------------------------------+



Spectral Index & Curvature
--------------------------

Measuring spectral terms
~~~~~~~~~~~~~~~~~~~~~~~~

Selavy is designed to work in conjunction with the ASKAPsoft pipeline. For continuum data, a common processing mode will be multi-frequency synthesis, where the output will be a series of "Taylor-term" images, being the coefficients of a Taylor-expansion of the frequency spectrum at each pixel. These images will be; taylor 0 - a total intensity (I) image (at a fiducial frequency); taylor 1 - the alpha * I map, where alpha is the spectral index; taylor 2 -  I * (beta + 0.5*alpha*(alpha-1)), where beta is the spectral curvature.

We want to extract a value for alpha & beta for each component. We do this by fitting to the total intensity image, as described above. Each resulting component is then fitted to the taylor 1 & 2 images, keeping the shape and location constant. This just fits the normalisation of the Gaussian. The total flux of the Gaussian is then extracted and used in the above relations.

The measurement of the spectral information in this way is dependent on the fitting, so one needs to request Gaussian fitting via the above parameters. The measurement of spectral index and spectral curvature can be requested independently (if, for instance, you have only a spectral index map).
 
Selavy defaults to assuming the images have been produced in the ASKAPsoft pipeline, and are thus named in a specific way. It is possible, however, to specify alternative names for the spectral index & curvature images (ie. taylor 1 & 2 maps), although the data they hold must be formed in the same way (ie. conform to the above relationships). The image names are specified via the **spectralTermImages** input parameter. If this is not given, the names are derived, if possible, from the image name, assuming a standard format: if the total intensity image is named XXX.taylor.0.YYY, then the spectral index map will be XXX.taylor.1.YYY and the spectral curvature map will be XXX.taylor.2.YYY


Parameters for spectral term measurement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+--------------------------+---------------+-------------------------------+----------------------------------------------------------------------+
| *Parameter*              | *Type*        | *Default*                     | *Explanation*                                                        |
+==========================+===============+===============================+======================================================================+
|Selavy.findSpectralTerms  |vector<bool>   |2 terms, same as Fitter.doFit  |A vector of 2 terms, indiciating whether to find the spectral index   |
|                          |               |                               |(first term) and the spectral curvature (2nd term). It is possible to |
|                          |               |                               |give only one term (e.g. findSpectralTerms = true) - then the second  |
|                          |               |                               |term will be set to false. To request both, put **findSpectralTerms = |
|                          |               |                               |[true,true]**.                                                        |
+--------------------------+---------------+-------------------------------+----------------------------------------------------------------------+
|Selavy.spectralTermImages |vector<string> |Derived from image name - see  |You can explicitly set the images for each term like so:              |
|                          |               |text                           |**spectralTermImages = [image1, image2]**.                            |
+--------------------------+---------------+-------------------------------+----------------------------------------------------------------------+
