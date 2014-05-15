Thresholds in Selavy
--------------------

The Duchamp package uses a single detection threshold for the entire image being searched. This can be either specified as a flux threshold **Selavy.threshold**, or as a signal-to-noise threshold via **Selavy.snrCut**. In the latter case, the noise is calculated from the image statistics over either the entire dataset or a subsection given by **Selavy.StatSec**.

However, if the sensitivity varies across the field, this will either mean some regions are not searched as deep as they could be and/or some are searched too deeply, resulting in too many spurious detections. Selavy deals with this in one of two ways.

The first is to use a weights image, such as that produced by the ASKAPsoft imager (and included in most of the ASKAP simulations), to scale the image according to the sensitivity. In practice, this takes the square root of the normalised weights and divides this into the pixel values. This has the effect of scaling down the low-sensitivity regions of the image, making it less likely that they present many spurious detections. Set **Selavy.WeightScaling=true** to utilise this mode - the weights image is specified via **Selavy.WeightScaling.weightsimage**, which has no default. **Note that the parameter interface governing this operation has recently changed.** The detection thresholds are provided in the usual fashion. The pixel values are only affected for the detection phase - parameter calculations are *not* affected.

The alternative is to impose a signal-to-noise threshold based on the *local* noise surrounding the pixel in question. This threshold then varies from pixel to pixel based on the change in the local noise. This mode is turned on using the **Selavy.VariableThreshold** parameter, which defaults to false. **Note that the parameter interface governing this operation has recently changed.**

This "local" level is estimated by measuring the noise properties of pixels within a box centred on the pixel in question. An array is thus built up containing the signal-to-local-noise values for each pixel in the image, and this array is then searched with a SNR threshold (**Selavy.snrCut**) and, if necessary, grown to a secondary SNR threshold (**Selavy.growthCut**). The way the noise properties are calculated is governed by the **Selavy.flagRobustStats** parameter. A value of *true* means robust statistics will be used, specifically the median and the median absolute deviation from the median (MADFM) -- the latter will be converted to the equivalent standard deviation for a Gaussian noise distribution for the purposes of calculating the signal-to-noise threshold. A value of *false* means we use the mean and the standard deviation. 

The searching can be done either spatially or spectrally, and this affects how the SNR values are calculated. If spatially (the default), a 2D sliding box filter is used to find the local noise. If spectrally, only a 1D "box" is used. Note that the edges (ie. all pixels within the half box width of the edge) are set to zero, and so detections will not be made there. This probably won't affect the 2D case, as often the edges of the field have poor sensitivity (certainly the ASKAP simulations mostly have a padding region around the edge), but in the 1D case this will mean the loss of the first & last channels. The choice between 2D and 1D is made with the **Selavy.searchType** parameter (which actually comes out of the Duchamp package).

When run on a distributed system as above, this processing is done at the worker level. Note that having an overlap between workers of at least the half box width will give continuous coverage (avoiding the aforementioned edge problems). The amount of processing needed increases quickly with the size of the box, due to the use of medians, particularly for the 2D case. 

A final option for varying the threshold spatially is to use a different threshold for each worker. In this scenario, switched on by setting **thresholdPerWorker = true**, each worker finds its own threshold based on the noise within it, using the **snrCut** signal-to-noise ratio threshold. No variation of the threshold *within* a worker is done, so you get discrete jumps in the threshold at worker boundaries. Use of the overlap can mitigate this. This mode was implemented more as an experiment than out of any expectation it would be useful, and limited trials indicate it's probably not much use. For completeness we include the parameter here. 

Threshold-related parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------------------------+------------+-------------+------------------------------------------------------------------+
|*Parameter*                       |*Type*      |*Default*    |*Description*                                                     |
+==================================+============+=============+==================================================================+
|Selavy.threshold                  |float       |no default   |The flux threshold applied to the entire image. Not compatible    |
|                                  |            |             |with the variable threshold parameters. If given, takes           |
|                                  |            |             |precendence over **Selavy.snrcut**.                               |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.snrCut                     |float       |5.           |The signal-to-noise threshold, in units of sigma above the mean.  |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.WeightScaling              |bool        |false        |Whether to scale the fluxes by the weights for the purposes of    |
|                                  |            |             |source detection.                                                 |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.WeightScaling.weightsimage |string      |""           |The filename of the weights image to be used to scale the fluxes  |
|                                  |            |             |prior to searching.                                               |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.VariableThreshold          |bool        |false        |If true, a sliding box function is used to find the local noise   |
|                                  |            |             |properties, which are used to make a signal-to-noise map that can |
|                                  |            |             |be used for searching.                                            |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.VariableThreshold.boxSize  |int         |50           |The half-width of the box used in the SNR map calculation. The    |
|                                  |            |             |full width of the box is 2*boxSize+1.                             |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.searchType                 |string      |spatial      |In which sense to do the searching: spatial=2D searches, one      |
|                                  |            |             |channel map at a time; spectral=1D searches, one spectrum at a    |
|                                  |            |             |time. The variable searches are affected by this, in that the     |
|                                  |            |             |spatial search uses a 2D box, while the spectral search uses a 1D |
|                                  |            |             |box.                                                              |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.flagRobustStats            |bool        |true         |Whether to calculate the noise properties with robust statistics  |
|                                  |            |             |(that is, the median and the median absolute deviation from the   |
|                                  |            |             |median), or (if false) the mean and standard deviation.           |
+----------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.thresholdPerWorker         |bool        |false        |If true, each worker's subimage sets its own threshold.           |
+----------------------------------+------------+-------------+------------------------------------------------------------------+

Saving threshold maps
~~~~~~~~~~~~~~~~~~~~~

Selavy provides the option of writing out the various arrays created for the median box searching. These include the signal-to-noise map, the noise map and the threshold map. These will be written to a CASA image. If the name is not given, no image will be written.

In a parallel case, the current behaviour is to write individual images for each worker, showing just their subsection of the full image. The name in this case will be modified to have a suffix like "_workerNum_numProcesses" (so that worker #6 of 8 will have the suffix "_6_8"). Due to the multiple outputs, these options are perhaps best used in serial mode (it is currently intended more for debugging and analysis purposes, and may be improved as we move towards actual operations). 

The parameters controlling this behaviour are listed below.

+--------------------------------------------+------------+-------------+------------------------------------------------------------------+
|*Parameter*                                 |*Type*      |*Default*    |*Description*                                                     |
+============================================+============+=============+==================================================================+
|Selavy.VariableThreshold.SNRimageName       |string      |""           |The name of the CASA image containing the SNR map                 |
|                                            |            |             |                                                                  |
+--------------------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.VariableThreshold.ThresholdImageName |string      |""           |The name of the CASA image containing the threshold map           |
|                                            |            |             |                                                                  |
+--------------------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.VariableThreshold.NoiseImageName     |string      |""           |The name of the CASA image containing the noise map               |
|                                            |            |             |                                                                  |
+--------------------------------------------+------------+-------------+------------------------------------------------------------------+
|Selavy.VariableThreshold.AverageImageName   |string      |""           |The name of the CASA image containing the background average map  |
|                                            |            |             |                                                                  |
+--------------------------------------------+------------+-------------+------------------------------------------------------------------+
