Preprocessing for Selavy
========================

A summary of the different types of preprocessing available to Selavy. This refers to work done on the image prior to source-finding, to enhance the detectability of objects.

A trous wavelet reconstruction
------------------------------



Smoothing
---------


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
