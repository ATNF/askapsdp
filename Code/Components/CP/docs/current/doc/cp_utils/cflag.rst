cflag (Flagging Utility) Documentation
=======================================

This page provides some details about the "cflag" program located in:

| ``$ASKAP_ROOT/trunk/Code/Components/CP/pipelinetasks/current/apps/cflag.cc``

The cflag pipeline task is responsible for both selection based and dynamic flagging
of visibilities. It takes as input a configuration file which specifies both the
dataset (Measurement Set) to be transformed, the flaggers to use and
parameters for these flaggers. Current supported strategies are:

- Selection (i.e. manual selection)
- Elevation thresholding
- Stokes-V thresholding

These flaggers are described in more detail below, along with the description of
parameters.

Running the program
-------------------

The code should be compiled with the ASKAPsoft build system:

::

   $ cd $ASKAP_ROOT
   $ rbuild Code/Components/CP/pipelinetasks/current

It can then be run with the following command, e.g.

::

   $ ${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/cflag.sh -c config.in


Configuration Parameters
------------------------

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


Stokes-V Flagging
~~~~~~~~~~~~~~~~~

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


Configuration Example
---------------------

This example demonstrates configuration of the Stokes-V (dynamic) flagger and the
selection based flagger with two rules specified.

.. code-block:: bash

    # The path/filename for the measurement set
    Cflag.dataset                           = target.ms

    # Enable Stokes V flagging flagger with a 5-sigma threshold
    Cflag.stokesv_flagger.enable            = true
    Cflag.stokesv_flagger.threshold         = 5.0

    # Elevation based flagging
    Cflag.elevation_flagger.enable          = true
    Cflag.elevation_flagger.low             = 12.0

    # Enable selection based flagging with two rules
    Cflag.selection_flagger.rules           = [rule1, rule2]

    # Selection Rule 1: Beams 0 and 1 on antenna "Pad01"
    Cflag.selection_flagger.rule1.antenna   = Pad01
    Cflag.selection_flagger.rule1.feed      = [0, 1]

    # Selection Rule 2: Spectral Channels 0 to 16 (inclusive) on spectral window 0
    Cflag.selection_flagger.rule2.spw       = 0:0~16
