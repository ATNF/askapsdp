ccalapply (Calibration Applicator) Documentation
================================================

This page provides instruction for using the ccalapply program. The purpose of
this software is to apply calibration parameters to Measurement Sets.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $ ccalapply -c config.in

The *ccalapply* program is not parallel/distributed, it runs in a single process operating
on a single input measurement set.

Configuration Parameters
------------------------

The following table contains the configuration parameters to be specified in the "config.in"
file shown on above command line. Note that each parameter must be prefixed with "Ccalapply.".
For example, the "dataset" parameter becomes "Ccalapply.dataset".

In addition to the below parameters, those described in :doc:`calibration_solutions`
are applicable. Specifically:

* Ccalapply.calibaccess
* Ccalapply.calibaccess.parset (if calibaccess is "parset"); or
* Ccalapply.calibaccess.table (if calibaccess is "table")

+--------------------------+------------------+--------------+----------------------------------------------------+
|**Parameter**             |**Type**          |**Default**   |**Description**                                     |
+==========================+==================+==============+====================================================+
|dataset                   |string            |None          |The name of the measurement set to which the        |
|                          |                  |              |calibration parameters will be applied.             |
+--------------------------+------------------+--------------+----------------------------------------------------+
|calibrate.scalenoise      |bool              |false         |If true, the noise estimate will be scaled in       |
|                          |                  |              |accordance with the applied calibrator factor to    |
|                          |                  |              |achieve proper weighting.                           |
+--------------------------+------------------+--------------+----------------------------------------------------+
|calibrate.allowflag       |bool              |false         |If true, corresponding visibilities are flagged if  |
|                          |                  |              |the inversion of Mueller matrix fails. Otherwise, an|
|                          |                  |              |exception is thrown should the matrix inversion fail|
+--------------------------+------------------+--------------+----------------------------------------------------+
|calibrate.ignorebeam      |bool              |false         |If true, the calibration solution corresponding to  |
|                          |                  |              |beam 0 will be applied to all beams                 |
+--------------------------+------------------+--------------+----------------------------------------------------+
|freqframe                 |string            |topo          |Frequency frame to work in (the frame is converted  |
|                          |                  |              |when the dataset is read). Either lsrk or topo is   |
|                          |                  |              |supported.                                          |
+--------------------------+------------------+--------------+----------------------------------------------------+

Example
-------

.. code-block:: bash

    Ccalapply.dataset                   = mydataset.ms

    Ccalapply.calibaccess               = table
    Ccalapply.calibaccess.table         = calparameters.tab
