Preprocessing for Selavy
========================

A summary of the different types of preprocessing available to Selavy. This refers to work done on the image prior to source-finding, to enhance the detectability of objects. Some of these algorithms are from the Duchamp source-finder, and are described in the `Duchamp User's Guide`_ and `Whiting (2012), MNRAS 421, 3242`_.

 .. _Duchamp User's Guide: http://www.atnf.csiro.au/Matthew.Whiting/Duchamp/DuchampUserGuide.pdf
 .. _Whiting (2012), MNRAS 421, 3242: http://onlinelibrary.wiley.com/doi/10.1111/j.1365-2966.2012.20548.x/full

À trous wavelet reconstruction
------------------------------

Algorithm details
~~~~~~~~~~~~~~~~~

One way Selavy can try to increase the apparent signal-to-noise of objects is to perform a thresholded multi-resolution wavelet reconstruction of the data, using the à trous algorithm. Briefly, the algorithm smoothes the data with progressively larger kernels (doubling the separation of the kernel elements each time), then forms wavelet arrays from the difference of successive smoothed arrays. These wavelet arrays describe the amount of signal at that scale at that location. Applying a threshold to these, and only keeping signal above the threshold, can reject a lot of the random noise present in the image. Once thresholded, the wavelet arrays are summed back together, then source extraction can be performed, generally to a quite low level. Wavelet reconstruction tends to give very good reliability.

The key parameters controlling the reconstruction are the dimensionality of the reconstruction, the level of the thresholding applied to the wavelet arrays, and the range of scales to make use of. There are other, more minor, parameters governing the specifics of the kernel, and the criterion for determining when to stop the reconstruction (looking at the degree of convergence, testing if there is still significant signal being missed).

Reconstruction parameters
~~~~~~~~~~~~~~~~~~~~~~~~~

Note that these are not in the usual hierarchical format (constrast with the 2D1D case below) -- this is because this algorithm originates with the Duchamp source finder, which has a flat parameter set input.

+------------------------+------------+------------+--------------------------------------------------------------+
|*Parameter*             |*Type*      |*Default*   |*Explanation*                                                 |
+========================+============+============+==============================================================+
|flagAtrous              |bool        |false       |Whether to use the wavelet reconstruction                     |
+------------------------+------------+------------+--------------------------------------------------------------+
|reconDim                |int         |1           |Dimension of reconstruction - how much is reconstructed at    |
|                        |            |            |once                                                          |
+------------------------+------------+------------+--------------------------------------------------------------+
|snrRecon                |float       |4           |Signal-to-noise threshold applied to wavelet arrays           |
+------------------------+------------+------------+--------------------------------------------------------------+
|scaleMin                |int         |1           |Minimum wavelet scale to include in reconstruction            |
+------------------------+------------+------------+--------------------------------------------------------------+
|scaleMax                |int         |0           |Maximum wavelet scale to use in the reconstruction. If 0 or   |
|                        |            |            |negative, then maximum scale is calculated from the size of   |
|                        |            |            |the array.                                                    |
+------------------------+------------+------------+--------------------------------------------------------------+
|filterCode              |int         |1           |Code number for the filter that defines the smoothing kernel. |
|                        |            |            |Consult the User Guide for details.                           |
+------------------------+------------+------------+--------------------------------------------------------------+
|reconConvergence        |float       |0.005       |The criterion for convergence of the reconstruction - the     |
|                        |            |            |relative changes in the standard devidation of the residuals  |
|                        |            |            |must be less than this.                                       |
+------------------------+------------+------------+--------------------------------------------------------------+

Smoothing
---------

Alternatively, the array can be smoothed by a filter of a single scale. This can be done either in the spatial domain or the spectral domain. If spatial, a two-dimensional Gaussian kernel is used. If spectral, a hanning-style filter of a defined width is used. Smoothing can provide catalogues with very good completeness, depending on the appropriateness of the filter.

+------------------------+------------+------------+-------------------------------------------------------------------+
|*Parameter*             |*Type*      |*Default*   |*Explanation*                                                      |
+========================+============+============+===================================================================+
|flagSmooth              |bool        |false       |Whether to use the wavelet reconstruction                          |
+------------------------+------------+------------+-------------------------------------------------------------------+
|smoothType              |string      |spectral    |The smoothing method used: either "spectral" or "spatial"          |
+------------------------+------------+------------+-------------------------------------------------------------------+
|hanningWidth            |int         |5           |The width of the Hanning spectral smoothing kernel                 |
+------------------------+------------+------------+-------------------------------------------------------------------+
|kernMaj                 |float       |3           |The FWHM of the major axis of the 2D Gaussian smoothing kernel, in |
|                        |            |            |pixels.                                                            |
+------------------------+------------+------------+-------------------------------------------------------------------+
|kernMin                 |float       |3           |The FWHM of the minor axis of the Gaussian kernel, in pixels       |
+------------------------+------------+------------+-------------------------------------------------------------------+
|kernPA                  |float       |0           |The position angle, in degrees, of the Gaussian kernel             |
+------------------------+------------+------------+-------------------------------------------------------------------+
|smoothEdgeMethod        |string      |equal       |The method for dealing with the pixels on the edge of the image    |
|                        |            |            |when doing the spatial smoothing. Can be one of: equal (all pixels |
|                        |            |            |treated the same), truncated (pixels near edge blanked out) or     |
|                        |            |            |scale (pixels near edge scaled by the relative amount of the       |
|                        |            |            |smoothing kernel used).                                            |
+------------------------+------------+------------+-------------------------------------------------------------------+
|spatialSmoothCutoff     |float       |1.e-10      |The value of the Gaussian kernel defining the width of the 2D      |
|                        |            |            |kernel array. Relative to the peak of 1.                           |
+------------------------+------------+------------+-------------------------------------------------------------------+



2D1D Wavelet reconstruction
---------------------------

Selavy also provides the ability to reconstruct the image using the 2D1D wavelet transform (see `Flöer & Winkel 2012, PASA 29, 244`_). This operates as an alternative reconstruction method to the Duchamp à trous algorithm, and works with the rest of the source finding in exactly the same way. The parameter-set interface is different, however, to better use the capabilities of the parameter set - see below. 

.. _Flöer & Winkel 2012, PASA 29, 244: http://adsabs.harvard.edu/abs/2012PASA...29..244F

Algorithm details
~~~~~~~~~~~~~~~~~

The version of the algorithm being used is a specially-constructed version provided by Lars Flöer, that has the same interface as the Duchamp wavelet algorithms. The parameters that are available to the user are similar to those required by the à trous algorithms. The key one is the reconstruction threshold **snrRecon**, that defines the n-sigma threshold used in keeping or rejecting wavelet coefficients. The minimum and maximum scales to be considered for the spatial and spectral directions can be specified (the defaults use all possible scales). The maximum number of iterations can be specified also (unlike the Duchamp algorithms, no convergence test is applied) - on subsequent iterations, the residual (input - output) is searched for further signal not yet recovered in the output array. There is also the option not in the Duchamp algorithms to enforce "positivity" in the final result - essentially setting negative values to zero.

2D1D parameters
~~~~~~~~~~~~~~~

+------------------------------+------------+------------+-------------------------------------------------------------+
|*Parameter*                   |*Type*      |*Default*   |*Explanation*                                                |
+==============================+============+============+=============================================================+
|recon2D1D                     |bool        |false       |Whether to use the 2D1D algorithm                            |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.snrRecon            |float       |3.0         |The signal-to-noise threshold applied to the wavelet         |
|                              |            |            |coefficients                                                 |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.useDuchampStats     |bool        |false       |Whether to find the stats in the same way as the Duchamp     |
|                              |            |            |algorithms.If false, the rms of the wavelet array is used in |
|                              |            |            |combination with snrRecon                                    |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.minXYscale          |int         |1           |The minimum spatial scale to be considered in the            |
|                              |            |            |reconstruction                                               |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.maxXYscale          |int         |-1          |The maximum spatial scale to be considered in the            |
|                              |            |            |reconstruction. If negative, the maximum possible is used -  |
|                              |            |            |ln_2(min(xdim,ydim))                                         |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.minZscale           |int         |1           |The minimum spectral scale to be considered in the           |
|                              |            |            |reconstruction                                               |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.maxZscale           |int         |-1          |The maximum spectral scale to be considered in the           |
|                              |            |            |reconstruction. If negative, the maximum possible is used -  |
|                              |            |            |ln_2(zdim)                                                   |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.enforcePositivity   |bool        |true        |Whether to set negative values in the output result to zero. |
|                              |            |            |                                                             |
+------------------------------+------------+------------+-------------------------------------------------------------+
|recon2D1D.maxIter             |int         |1           |The maximum number of iterations of the algorithm            |
+------------------------------+------------+------------+-------------------------------------------------------------+
