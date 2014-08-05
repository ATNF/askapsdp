cflag (Flagging Utility)
========================

The cflag pipeline task is responsible for both selection based and dynamic flagging
of visibilities. It takes as input a configuration file which specifies both the
dataset (Measurement Set) to be transformed, the flaggers to use and
parameters for these flaggers. Current supported strategies are:

- Selection (i.e. manual selection)
- Elevation thresholding
- Stokes-V thresholding
- Amplitude thresholding

These flaggers are described in more detail below, along with the description of
parameters.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $ cflag -c config.in

The *cflag* program is not parallel/distributed, it runs in a single process operating
on a single input measurement set.

Configuration Parameters
------------------------

An example configuration parameter set is provided in the `Configuration Example`_
section.

+----------------------+------------+-----------------------+---------------------------------------------+
|*Parameter*           |*Default*   |*Example*              |*Description*                                |
+======================+============+=======================+=============================================+
|Cflag.dataset         |*None*      |fornax.ms              |The measurement set (uv-dataset) to be       |
|                      |            |                       |flagged. This file has flagging applied in   |
|                      |            |                       |place (i.e. it is modified)                  |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cflag.dryrun          |false       |true                   |If set to true, the dataset will not be      |
|                      |            |                       |modified. The flaggers will still report     |
|                      |            |                       |flagging information so the user can see what|
|                      |            |                       |flagging would have taken place if "dryrun"  |
|                      |            |                       |was set to false.                            |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cflag.summary         |true        |false                  |If "true" then a summary of the measurement  |
|                      |            |                       |set is displayed before flagging. This       |
|                      |            |                       |contains information such as previous        |
|                      |            |                       |flagging. However, an extra pass over the    |
|                      |            |                       |data is done, so for very large measurement  |
|                      |            |                       |sets this can be avoided by setting this     |
|                      |            |                       |parameter to "false"                         |
+----------------------+------------+-----------------------+---------------------------------------------+
    
Selection Base Flagging
~~~~~~~~~~~~~~~~~~~~~~~

A selection based flagging flagger. This allows flagging based on:
- Baseline (i.e. an antenna or a pair of antennas)
- Field index number
- Time range
- Scan index number
- Feed/beam index number
- UVRange
- Autocorrelations only
- Spectral Window (e.g. channel index number or frequency)

+------------------------------------------+---------+----------------+-----------------------------------+
|*Parameter*                               |*Default*|*Example*       |*Description*                      |
+==========================================+=========+================+===================================+
|Cflag.selection_flagger.rules             |None     |[rule1, rule2]  |The list of rules for selection    |
|                                          |         |                |based flagging. If this parameter  |
|                                          |         |                |is not specified, the selection    |
|                                          |         |                |based flagger is not used.         |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.field      |*None*   |See below URL   |Flag based on field index number   |
|                                          |         |                |                                   |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.spw        |*None*   |See below URL   |Flag based on spectral window      |
|                                          |         |                |                                   |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.antenna    |*None*   |See below URL   |Flag based on an antenna or antenna|
|                                          |         |                |pair                               |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.timerange  |*None*   |See below URL   |Flag based on a time range         |
|                                          |         |                |                                   |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.correlation|*None*   |See below URL   |Flag specific correlation products |
|                                          |         |                |                                   |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.scan       |*None*   |See below URL   |Flag all rows in a given scan,     |
|                                          |         |                |based on scan index number         |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.feed       |*None*   |[0, 1]          |An array of beam index numbers to  |
|                                          |         |                |flag.                              |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.uvrange    |*None*   |See below URL   |Flag all baselines for a given UV  |
|                                          |         |                |distance range                     |
+------------------------------------------+---------+----------------+-----------------------------------+
|Cflag.selection_flagger.<rule>.autocorr   |false    |true            |Flag auto correlations             |
+------------------------------------------+---------+----------------+-----------------------------------+

Selection syntax is described here: http://www.aoc.nrao.edu/~sbhatnag/misc/msselection/msselection.html


Elevation Thresholding
~~~~~~~~~~~~~~~~~~~~~~

This flagger will flag any visibilities where one or both of the antennas have
an elevation either lower than the "low" threshold or higher than the "high"
threshold. This allows flagging when the antennas are pointed either near
the horizon or the zenith.

+----------------------------------+------------+------------+---------------------------------------------+
|*Parameter*                       |*Default*   |*Example*   |*Description*                                |
+==================================+============+============+=============================================+
|Cflag.elevation_flagger.enable    |false       |true        |Enable the elevation thresholding based      |
|                                  |            |            |flagging                                     |
+----------------------------------+------------+------------+---------------------------------------------+
|Cflag.elevation_flagger.low       |0.0         |10.0        |Defines the lower threshold (in degrees). All|
|                                  |            |            |visibilities for which the elevation was     |
|                                  |            |            |lowever than this threshold will be flagged. |
+----------------------------------+------------+------------+---------------------------------------------+
|Cflag.elevation_flagger.high      |90.0        |89.5        |Defines the upper threshold (in degrees). All|
|                                  |            |            |visibilities for which the elevation was     |
|                                  |            |            |higher than this threshold will be flagged.  |
+----------------------------------+------------+------------+---------------------------------------------+


Stokes-V Thresholding
~~~~~~~~~~~~~~~~~~~~~

Performs flagging based on Stokes-V thresholding. For each row the mean
and standard deviation for all Stokes-V correlations (i.e. all channels
within a given row) are calculated. Then, where the Stokes-V correlation
exceeds the average plus (stddev * threshold) all correlations for that
channel in that row will be flagged.

+----------------------------------+------------+------------+---------------------------------------------+
|*Parameter*                       |*Default*   |*Example*   |*Description*                                |
+==================================+============+============+=============================================+
|Cflag.stokesv_flagger.enable      |false       |true        |Enable the Stokes-V dynamic flagging         |
+----------------------------------+------------+------------+---------------------------------------------+
|Cflag.stokesv_flagger.threshold   |5.0         |5.0         |The threshold at which visibilities will be  |
|                                  |            |            |flagged. Where the amplitude of a correlation|
|                                  |            |            |exceeds the (average + (stddev * threshold)) |
|                                  |            |            |all correlations for that spectral channel in|
|                                  |            |            |the row will be flagged.                     |
+----------------------------------+------------+------------+---------------------------------------------+

Amplitude Thresholding 
~~~~~~~~~~~~~~~~~~~~~~

The "amplitude thresholding" flagger is a very basic flagger used to flag visibilities
which fall outside some amplitude bounds. This was designed for ASKAP commissioning to
potentially work around some correlator problems.

+--------------------------------------------------+------------+------------+---------------------------------------------+
|*Parameter*                                       |*Default*   |*Example*   |*Description*                                |
+==================================================+============+============+=============================================+
|Cflag.amplitude_flagger.enable                    |false       |true        |Enable amplitude threshold flagging          |
+--------------------------------------------------+------------+------------+---------------------------------------------+
|Cflag.amplitude_flagger.low                       |*None*      |1e-17       |The lower bound for valid visibilities. Any  |
|                                                  |            |            |visibility with a lower amplitude will be    |
|                                                  |            |            |flagged. If this parameter is not present in |
|                                                  |            |            |the parset, then no lower bound will be      |
|                                                  |            |            |enforced.                                    |
+--------------------------------------------------+------------+------------+---------------------------------------------+
|Cflag.amplitude_flagger.high                      |*None*      |12345.0     |The upper bound for valid visibilities. Any  |
|                                                  |            |            |visibility with a higher amplitude will be   |
|                                                  |            |            |flagged. If this parameter is not present in |
|                                                  |            |            |the parset, then no upper bound will be      |
|                                                  |            |            |enforced.                                    |
+--------------------------------------------------+------------+------------+---------------------------------------------+
|Cflag.amplitude_flagger.stokes                    |*None*      |[XX, YY]    |Specifies which correlation products are to  |
|                                                  |            |            |be subject to flagging. If this parameter is |
|                                                  |            |            |not specified then **all** products will be  |
|                                                  |            |            |subject to flagging. To just flag XX, then   |
|                                                  |            |            |specify "[XX]". For XX & YY, "[XX, YY]", and |
|                                                  |            |            |so on. No stokes conversion is done, so only |
|                                                  |            |            |the products contained in the measurement set|
|                                                  |            |            |should be specified.                         |
+--------------------------------------------------+------------+------------+---------------------------------------------+
|Cflag.amplitude_flagger.autoThresholds            |false       |true        |If true, automatically generate low and high |
|                                                  |            |            |amplitude thresholds for each spectrum using |
|                                                  |            |            |the statistics described below. Both         |
|                                                  |            |            |Cflag.amplitude_flagger.low and              |
|                                                  |            |            |Cflag.amplitude_flagger.high have preference |
|                                                  |            |            |over the autoThresholds.                     |
+--------------------------------------------------+------------+------------+---------------------------------------------+
Cflag.amplitude_flagger.threshold                  |5.0         |4.0         |The threshold factor used in the statistics  |
|                                                  |            |            |described below.                             |
+--------------------------------------------------+------------+------------+---------------------------------------------+
Cflag.amplitude_flagger.integrateSpectra           |false       |true        |Integrate the spectra in time and flag any   |
|                                                  |            |            |channels outside thresholds, also set using  |
|                                                  |            |            |the statistics described below. Spectra for  |
|                                                  |            |            |different baselines, beams, fields and       |
|                                                  |            |            |polarisation are kept separate. Requires a   |
|                                                  |            |            |second pass over the data.                   |
+--------------------------------------------------+------------+------------+---------------------------------------------+
Cflag.amplitude_flagger.integrateSpectra.threshold |5.0         |4.0         |The threshold factor used to threshold       |
|                                                  |            |            |integrated spectra.                          |
+--------------------------------------------------+------------+------------+---------------------------------------------+
Cflag.amplitude_flagger.aveAll                     |false       |true        |Do not separate spectra based on baseline,   |
|                                                  |            |            |etc., when integrating spectra. Average      |
|                                                  |            |            |everything together.                         |
+--------------------------------------------------+------------+------------+---------------------------------------------+
Cflag.amplitude_flagger.aveAll.noPol               |false       |true        |Do separate spectra for different            |
|                                                  |            |            |polarisations.                               |
+--------------------------------------------------+------------+------------+---------------------------------------------+
Cflag.amplitude_flagger.aveAll.noBeam              |false       |true        |Do separate spectra for different beams.     |
+--------------------------------------------------+------------+------------+---------------------------------------------+

To avoid additional passes over data containing RFI spikes, the median and interquartile range are used in
place of the mean and standard deviation used in many thresholding algorithms. These are more robust to a
modest number of outliers. If Gaussian noise dominates most of the frequency channels, then ~50% of the
amplitudes will lie within 0.674 sigma of the mean, such that sigma ~ 1.349*IQL (IQL = the interquartile
range). Samples outside [median - thresholdFactor*sigma, median + thresholdFactor*sigma] are flagged.

Configuration Example
---------------------

**Example 1**

This example demonstrates configuration of the Stokes-V (dynamic) flagger and the
selection based flagger with two rules specified:

.. code-block:: bash

    # The path/filename for the measurement set
    Cflag.dataset                           = target.ms

    # Enable Stokes V flagging flagger with a 5-sigma threshold
    Cflag.stokesv_flagger.enable            = true
    Cflag.stokesv_flagger.threshold         = 5.0

    # Enable selection based flagging with two rules
    Cflag.selection_flagger.rules           = [rule1, rule2]

    # Selection Rule 1: Beams 0 and 1 on antenna "ak01"
    Cflag.selection_flagger.rule1.antenna   = ak01
    Cflag.selection_flagger.rule1.feed      = [0, 1]

    # Selection Rule 2: Spectral Channels 0 to 16 (inclusive) on spectral window 0
    Cflag.selection_flagger.rule2.spw       = 0:0~16


**Example 2**

This example demonstrates configuration of the elevation flagger and the amplitude based
flagger with both a low and high threshold:

.. code-block:: bash

    # The path/filename for the measurement set
    Cflag.dataset                           = target.ms

    # Elevation based flagging
    Cflag.elevation_flagger.enable          = true
    Cflag.elevation_flagger.low             = 12.0
    Cflag.elevation_flagger.high            = 89.0

    # Amplitude based flagging
    Cflag.amplitude_flagger.enable          = true
    Cflag.amplitude_flagger.high            = 10.25
    Cflag.amplitude_flagger.low             = 1e-3

**Example 3**

This example demonstrates configuration of the amplitude based
flagger with dynamic thresholding:

.. code-block:: bash

    # The path/filename for the measurement set
    Cflag.dataset                                      = target.ms
    # Amplitude based flagging
    Cflag.amplitude_flagger.enable                     = true
    # Threshold using the median and IQR of each spectrum
    Cflag.amplitude_flagger.autoThresholds             = true
    # Threshold again after averaging spectra in time
    Cflag.amplitude_flagger.integrateSpectra           = true
    Cflag.amplitude_flagger.integrateSpectra.threshold = 4.0

