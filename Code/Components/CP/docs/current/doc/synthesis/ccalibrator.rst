ccalibrator Documentation
=========================

This page provides some details about the ccalibrator program. The purpose of this software is to perform calibration in a parallel/distributed environment or on a single computer system. The software leverages MPI, however can be run on a simple laptop or a large supercomputer.

Parset parameters understood by ccalibrator are given in the following table (all parameters must have **Ccalibrator** prefix, i.e. **Ccalibrator.dataset**). For a number of parameters certain keywords are substituted, i.e. **%w** is replaced by the rank and **%n** by the number of nodes in the parallel case. In the serial case, these special strings are substituted by 0 and 1, respectively. This substitution allows to reuse the same parameter file on all nodes of the cluster if the difference between jobs assigned to 
individual nodes can be coded by using these keywords (e.g. using specially crafted file names). If a parameter supports substitution, it is clearly stated in the description. 

The **ccalibrator** is intended to process data from a sufficiently short timescale, where calibratable unknowns can be assumed to be constant
(in the ASKAPsoft approach these corrections are applied upstream and each calibration cycle corresponds to a separate execution of the 
calibration code. At this stage only antenna and beam dependent gain (without cross-polarisation terms) can be calibrated. Note, that the code is still experimental and has a number of parameters hard coded.

The output file with the result has a parset format understood by **Cimager**. This output file is called **result.dat** and has complex-valued (stored as 2-element double vectors of real and imaginary parts) keywords named like **gain.g11.x.b** and **gain.g22.y.c**, where **x** and **y** are 0-based antenna numbers, **b** and **c** are 0-based beam numbers and **g11**, **g22** corresponds to the first and second polarisations (in the frame of the measurement). 
 
A number of other parameters allowing to narrow down the data selection are understood. They are given in [:doc:`data_selection`] and should also have the **Ccalibrator** prefix.

+-----------------------+----------------+--------------+-------------------------------------------------+
|**Parameter**          |**Type**        |**Default**   |**Description**                                  |
+=======================+================+==============+=================================================+
|imagetype              |string          |"casa"        |Type of the image handler (determines the format |
|                       |                |              |of the images read from the disk). The default is|
|                       |                |              |to read casa format and this is the only option  |
|                       |                |              |implemented so far.                              |
+-----------------------+----------------+--------------+-------------------------------------------------+
|nAnt                   |uint            |36            |Number of antennas to solve the gains for. The   |
|                       |                |              |code will fail if it is requested to solve for   |
|                       |                |              |more antennas than it has the data for           |
+-----------------------+----------------+--------------+-------------------------------------------------+
|nBeam                  |uint            |1             |Number of beams to solve the gains for. The code |
|                       |                |              |will fail if it is requested to solve for more   |
|                       |                |              |beams than it has the data for                   |
+-----------------------+----------------+--------------+-------------------------------------------------+
|interval               |quantity string |"-1s"         |If a positive number is given, a separate        |
|                       |                |              |calibration solution will be made for each chunk |
|                       |                |              |of visibilities obtained within the time interval|
|                       |                |              |equal to the value of this parameter. For a      |
|                       |                |              |negative value, a single solution is made for the|
|                       |                |              |whole dataset                                    |
+-----------------------+----------------+--------------+-------------------------------------------------+
|dataset                |string or       |None          |Data set file name to produce. Usual substitution|
|                       |vector<string>  |              |rules apply if the parameter is a single         |
|                       |                |              |string. If the parameter is given as a vector of |
|                       |                |              |strings all measurement sets given by this vector|
|                       |                |              |are effectively concatenated together on-the-fly |
|                       |                |              |in the serial case. In the parallel case, the    |
|                       |                |              |size of the vector is required to be either 1 or |
|                       |                |              |the number of nodes - 1, and therefore there is  |
|                       |                |              |one measurement set per worker node.             |
+-----------------------+----------------+--------------+-------------------------------------------------+
|datacolumn             |string          |"DATA"        |The name of the data column in the measurement   |
|                       |                |              |set which will be the source of visibilities.This|
|                       |                |              |can be useful to process real telescope data     |
|                       |                |              |which were passed through **casapy** at some     |
|                       |                |              |stage (e.g. to run on calibrated data which are  |
|                       |                |              |stored in the **CORRECTED_DATA** column). In the |
|                       |                |              |measurement set convention, the **DATA** column  |
|                       |                |              |which is used by default contains raw            |
|                       |                |              |uncalibrated data as received directly from the  |
|                       |                |              |telescope. Calibration tasks in **casapy** make a|
|                       |                |              |copy when calibration is applied creating a new  |
|                       |                |              |data column.                                     |
+-----------------------+----------------+--------------+-------------------------------------------------+
|nUVWMachines           |int32           |1             |Size of uvw-machines cache. uvw-machines are used|
|                       |                |              |to convert uvw from a given phase centre to a    |
|                       |                |              |common tangent point. To reduce the cost to set  |
|                       |                |              |the machine up (calculation of the transformation|
|                       |                |              |matrix), a number of these machines is           |
|                       |                |              |cached. The key to the cache is a pair of two    |
|                       |                |              |directions: the current phase centre and the     |
|                       |                |              |tangent centre. If the required pair is within   |
|                       |                |              |the tolerances of that used to setup one of the  |
|                       |                |              |machines in the cache, this machine is reused. If|
|                       |                |              |none of the cache items matches the least        |
|                       |                |              |accessed one is replaced by the new machine which|
|                       |                |              |is set up with the new pair of directions. The   |
|                       |                |              |code would work faster if this parameter is set  |
|                       |                |              |to the number of phase centres encountered during|
|                       |                |              |imaging. In non-faceting case, the optimal       |
|                       |                |              |setting would be the number of synthetic beams   |
|                       |                |              |times the number of fields. For faceting (btw,   |
|                       |                |              |the performance gain is quite significant in this|
|                       |                |              |case), it should be further multiplied by the    |
|                       |                |              |number of facets. Direction tolerances are given |
|                       |                |              |as a separate parameter.                         |
+-----------------------+----------------+--------------+-------------------------------------------------+
|uvwMachineDirTolerance |quantity string |"1e-6rad"     |Direction tolerance for the management of the    |
|                       |                |              |uvw-machine cache (see **nUVWMachines** for      |
|                       |                |              |details). The value should be an angular         |
|                       |                |              |quantity. The default value corresponds roughly  |
|                       |                |              |to 0.2 arcsec and seems sufficient for all       |
|                       |                |              |practical applications within the scope of       |
|                       |                |              |ASKAPsoft.                                       |
+-----------------------+----------------+--------------+-------------------------------------------------+
|refgain                |string          |""            |If not an empty string, this is assumed to be the|
|                       |                |              |name of the reference gain parameter (and so it  |
|                       |                |              |must exist, otherwise an exception will be       |
|                       |                |              |thrown), i.e. **gain.g11.0.0**. All phases in the|
|                       |                |              |resulting gains are rotated, so the reference    |
|                       |                |              |gain has the zero phase.                         |
+-----------------------+----------------+--------------+-------------------------------------------------+
|solve                  |string          |"gains"       |String describing what to solve for              |
|                       |                |              |(e.g. "gains,leakages" or "leakages" or          |
|                       |                |              |"gains"). If "antennagains" is used instead of   |
|                       |                |              |"gains", beam-independent gains are solved       |
|                       |                |              |for. Such a solution is stored as beam=0         |
|                       |                |              |solution. Use **calibrate.ignorebeam=true**      |
|                       |                |              |option of cimager to apply such beam-independent |
|                       |                |              |solution.                                        |
+-----------------------+----------------+--------------+-------------------------------------------------+
|sources.definition     |string          |None          |Optional parameter. If defined, sky model        |
|                       |                |              |(i.e. source info given as **sources.something**)|
|                       |                |              |is read from a separate parset file (name is     |
|                       |                |              |given by this parameter). If this parameter is   |
|                       |                |              |not defined, source description should be given  |
|                       |                |              |in the main parset file. Usual substitution rules|
|                       |                |              |apply. The parameters to define sky model are    |
|                       |                |              |described in :doc:`csimulator` (with Ccalibrator |
|                       |                |              |prefix instead of Csimulator)                    |
+-----------------------+----------------+--------------+-------------------------------------------------+
|gridder                |string          |None          |Name of the gridder, further parameters are given|
|                       |                |              |by **gridder.something**. See :doc:`gridder` for |
|                       |                |              |details.                                         |
+-----------------------+----------------+--------------+-------------------------------------------------+
|rankstoringcf          |int             |1             |In the parallel mode, only this rank will attempt|
|                       |                |              |to export convolution functions if this operation|
|                       |                |              |is requested (see **tablename** option in the    |
|                       |                |              |:doc:`gridder`). This option is ignored in the   |
|                       |                |              |serial mode.                                     |
+-----------------------+----------------+--------------+-------------------------------------------------+
|visweights             |string          |""            |If this parameter is set to "MFS" gridders are   |
|                       |                |              |setup to degrid with the weight required for the |
|                       |                |              |models given as Taylor series                    |
|                       |                |              |(i.e. multi-frequency synthesis models). At the  |
|                       |                |              |moment, this parameter is decoupled from the     |
|                       |                |              |setup of the model parameters. The user has to   |
|                       |                |              |set it separately and in a consistent way with   |
|                       |                |              |the model setup (the **nterms** parameter in the |
|                       |                |              |model definition (see :doc:`csimulator` for more |
|                       |                |              |details) should be set to something greater than |
|                       |                |              |1 and there should be an appropriate number of   |
|                       |                |              |models defined).                                 |
+-----------------------+----------------+--------------+-------------------------------------------------+
|visweights.MFS.reffreq |double          |1.405e9       |Reference frequency in Hz for MFS-model          |
|                       |                |              |simulation (see above)                           |
+-----------------------+----------------+--------------+-------------------------------------------------+
|ncycles                |int32           |1             |Number of solving iterations (and iterations over|
|                       |                |              |the dataset, which can be called major cycles,   |
|                       |                |              |although we don't do any minor cycles for        |
|                       |                |              |calibration)                                     |
+-----------------------+----------------+--------------+-------------------------------------------------+
|freqframe              |string          |topo          |Frequency frame to work in (the frame is         |
|                       |                |              |converted when the dataset is read). Either lsrk |
|                       |                |              |or topo is supported.                            |
+-----------------------+----------------+--------------+-------------------------------------------------+


The resulting parameters are stored into a solution source (or sink to be exact) as described in :doc:`calibration_solutions`

Example
-------

::

    Ccalibrator.dataset                                     = 10uJy_simtest.ms
    Ccalibrator.refgain                                     = gain.g11.0.0

    Ccalibrator.sources.names                               = [10uJy,field2]
    Ccalibrator.sources.10uJy.direction                     = [12h30m00.000, -45.00.00.000, J2000]
    Ccalibrator.sources.10uJy.model                         = 10uJy.model.small
    # phase centre is not handled properly in the components code, specify the offsets here
    Ccalibrator.sources.field2.direction                    = [12h30m00.000, -45.00.00.000, J2000]
    Ccalibrator.sources.field2.components                   = [src1]
    Ccalibrator.sources.src1.flux.i                         = 0.091
    Ccalibrator.sources.src1.direction.ra                   = 0.00363277
    Ccalibrator.sources.src1.direction.dec                  = -0.00366022

    Ccalibrator.gridder                                     = AProjectWStack
    Ccalibrator.gridder.AProjectWStack.wmax                 = 15000
    Ccalibrator.gridder.AProjectWStack.nwplanes             = 1
    Ccalibrator.gridder.AProjectWStack.oversample           = 4
    Ccalibrator.gridder.AProjectWStack.diameter             = 12m
    Ccalibrator.gridder.AProjectWStack.blockage             = 2m
    Ccalibrator.gridder.AProjectWStack.maxfeeds             = 2
    Ccalibrator.gridder.AProjectWStack.maxsupport           = 1024
    Ccalibrator.gridder.AProjectWStack.frequencydependent   = false

    Ccalibrator.ncycles                                     = 5
