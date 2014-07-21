extractdata
============

Usage::

    extractdata.sh -c extractdata.parset [measurement_set]

This utility makes a "waterfall" plot out of the dataset in both casa image
and fits format. It produces a cube with spectral channels in horizontal axis,
correlator cycles in vertical axis and baselines/beams as the planes of the cube.
The exact order of baselines and beams matches that in the measurement set and is
reported to the log. The waterfall plot can be created for amplitude, phase as well
as a real part. Each spectrum could optionally be converted into the 
lag domain by FFT and some averaging of consecutive integration cycles can also be made
by request. Only single selected polarisation product can be displayed. 

The measurement set can be given either in the command line, or in the parset 
(see below), but not both. The tool first creates a casa image cube, named result.img,
and then converts it to a fits cube (called result.fits). 

Parset parameters are described in the following table and should not contain any prefix
A number of other parameters allowing to narrow down the data selection are understood.
They are given in a separate table (see :doc:`data_selection`). For example, one can
select only the first beam (with **Feed=0**) and reduced the number of planes in the
resulting cube and its size.

+------------------------------+-------------+--------------------+-----------------------------------------+
|*Parameter*                   |*Type*       |*Default*           |*Description*                            |
+==============================+=============+====================+=========================================+
|stokes                        |string       |"XX"                |Polarisation product to use. The code    |
|                              |             |                    |does no conversion, so the parameter     |
|                              |             |                    |should correspond to a product which     |
|                              |             |                    |actually has been observed.
+------------------------------+-------------+--------------------+-----------------------------------------+
|dataset                       |string       |""                  |A path to the measurement set to use     |
|                              |             |                    |(required if ms is not given in the      |
|                              |             |                    |command line)                            |
+------------------------------+-------------+--------------------+-----------------------------------------+
|dofft                         |bool         |false               |If true, each spectrum is FFT'ed before  |
|                              |             |                    |selected product is extracted. This makes|
|                              |             |                    |the lag spectrum.                        |
+------------------------------+-------------+--------------------+-----------------------------------------+
|nAvg                          |uint32       |1                   |Number of consecutive integration cycles |
|                              |             |                    |to average before extracting data        |
+------------------------------+-------------+--------------------+-----------------------------------------+
|maxcycles                     |uint32       |2000                |Maximum number of cycles (used to create |
|                              |             |                    |buffers up front), should be greater than|
|                              |             |                    |obs. time / 5s. Note, if you run the tool|
|                              |             |                    |at MRO and have a large buffer it may    |
|                              |             |                    |affect observing and other users.        |
+------------------------------+-------------+--------------------+-----------------------------------------+
|datatype                      |string       |"amplitude"         |What to export. Available options are    |
|                              |             |                    |"amplitude", "phase", "real"             |
+------------------------------+-------------+--------------------+-----------------------------------------+
|makediff                      |bool         |false               |If true, the resulting cube will contain |
|                              |             |                    |differences between adjacent integration |
|                              |             |                    |cycles instead of the original data.     |
|                              |             |                    |This option is handy to investigate rates|
|                              |             |                    |of change or artefacts related to double |
|                              |             |                    |buffering, i.e. appearing every second   |
|                              |             |                    |cycle.                                   |
+------------------------------+-------------+--------------------+-----------------------------------------+
|zeroflags                     |bool         |false               |If true, flagged data are replaced by    |
|                              |             |                    |zeros before processing. Otherwise, the  |
|                              |             |                    |flag information is ignored.             |
+------------------------------+-------------+--------------------+-----------------------------------------+
|padding                       |uint32       |1                   |Pad spectra with zeros with this factor. |
|                              |             |                    |This is sometimes handy if lag spectrum  |
|                              |             |                    |at higher resolution is required (i.e.   |
|                              |             |                    |dofft=true)                              |
+------------------------------+-------------+--------------------+-----------------------------------------+



Examples
--------

**Example 1:**

The tool is used with the name of the measurement set given explicitly as a command line parameter

.. code-block:: bash

    stokes = XX
    # only show the first beam
    Feed = 0
    # exclude autocorrelations
    CorrelationType = cross
    # work in visibility domain
    dofft = false
    # no additional averaging
    nAvg = 1
    # buffer size
    maxcycles = 2000
    # export amplitudes
    datatype = amplitude

