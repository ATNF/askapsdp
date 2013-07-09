cflag (Flagging Utility) Documentation
=======================================

This page provides some details about the "cflag" program located in:

| ``$ASKAP_ROOT/trunk/Code/Components/CP/pipelinetasks/current/apps/cflag.cc``

The cflag pipeline task is responsible for both selection based and dynamic flagging
of visibilities. It takes as input a configuration file which specifies both the
dataset (Measurement Set) to be transformed and the flagging strategies and
parameters for these strategies. Current supported strategies are:

- Selection (i.e. manual selection)
- Stokes-V thresholding

These strategies are described in more detail below, along with the description of
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
|                      |            |                       |modified. The strategies will still report   |
|                      |            |                       |flagging information so the user can see what|
|                      |            |                       |flagging would have taken place if "dryrun"  |
|                      |            |                       |was set to false.                            |
+----------------------+------------+-----------------------+---------------------------------------------+
    
Selection Base Flagging
~~~~~~~~~~~~~~~~~~~~~~~

A selection based flagging strategy. This allows flagging based on:
- Baseline (i.e. an antenna or a pair of antennas)
- Field index number
- Time range
- Scan index number
- Feed/beam index number
- UVRange (not yet impelemented)
- Autocorrelations only
- Spectral (e.g. channel index number or frequency)

+--------------------------+---------------+------------------+-------------------------------------+
|*Parameter*               |*Default*      |*Example*         |*Description*                        |
+==========================+===============+==================+=====================================+
|Cflag.selection_flagger.r\|None           |[rule1, rule2]    |The list of rules for selection based|
|ules                      |               |                  |flagging. If this parameter is not   |
|                          |               |                  |specified, the selection based       |
|                          |               |                  |flagger is not used.                 |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.field                |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.spw                  |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.antenna              |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.timerange            |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.correlation          |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.scan                 |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.feed                 |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.uvrange              |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.selectin_flagger.<r\|*None*         |                  |                                     |
|ule>.autocorr             |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+


Stokes-V Flagging
~~~~~~~~~~~~~~~~~

Performs flagging based on Stokes-V thresholding. For each row the mean
and standard deviation for all Stokes-V correlations (i.e. all channels
within a given row) are calculated. Then, where the Stokes-V correlation
exceeds the average plus (stddev * threshold) all correlations for that
channel in that row will be flagged.

+--------------------------+---------------+------------------+-------------------------------------+
|*Parameter*               |*Default*      |*Example*         |*Description*                        |
+==========================+===============+==================+=====================================+
|Cflag.stokesv_strategy.en\|false          |true              |Enable the Stokes-V dynamic flagging |
|able                      |               |                  |strategy                             |
|                          |               |                  |                                     |
+--------------------------+---------------+------------------+-------------------------------------+
|Cflag.stokesv_strategy.th\|5.0            |5.0               |The threshold at which visibilities  |
|reshold                   |               |                  |will be flagged. Where a correlations|
|                          |               |                  |amplitude exceeds the average plus   |
|                          |               |                  |(stddev * threshold) all correlations|
|                          |               |                  |for that spectral channel in the row |
|                          |               |                  |will be flagged.                     |
+--------------------------+---------------+------------------+-------------------------------------+


Configuration Example
---------------------

This example demonstrates configuration of the Stokes-V (dynamic) flagger and the
selection based flagger with two rules specified.

.. code-block:: bash

    # The path/filename for the measurement set
    Cflag.dataset                           = target.ms

    # Enable Stokes V flagging strategy with a 5-sigma threshold
    Cflag.stokesv_strategy.enable           = true
    Cflag.stokesv_strategy.threshold        = 5.0

    # Enable selection based flagging with two rules
    Cflag.selection_flagger.rules           = [rule1, rule2]

    # Selection Rule 1: Beams 0 and 1 on antenna "Pad01"
    Cflag.selection_flagger.rule1.antenna   = Pad01
    Cflag.selection_flagger.rule1.feed      = [0, 1]

    # Selection Rule 2: Spectral Channels 0 to 16 (inclusive) on spectral window 0
    Cflag.selection_flagger.rule2.spw        = 0:0~16
