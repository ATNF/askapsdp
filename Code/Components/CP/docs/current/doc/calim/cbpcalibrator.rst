cbpcalibrator Documentation
=========================

The ccalibrator program performs calibration in a parallel/distributed environment
or on a single computer system. The software leverages MPI, however can be run on
a simple laptop or a large supercomputer. 

The cbpcalibrator program is a specialised tool for bandpass calibration. It is closely
related to **ccalibrator** (see :doc:`ccalibrator`) and performs one of its functions,
bandpass calibration, in an optimised fashion. The program can be run in a parallel/distributed
environment or on a single computer system. It automatically distributes the work between
multiple MPI ranks, if used on a supercomputer, with no parameter change required for the user. 
Unlike **ccalibrator**, this tool 

      * solves for bandpass only
      * works only with preaveraging calibration approach
      * does not support multiple chunks in time (i.e. only one solution is made for the whole dataset)
      * does not support data distribution except per beam 
      * does not support a distributed model (e.g. with individual workers dealing with individual Taylor terms)
      * does not require exact match between number of workers and number of channel chunks, data are dealt with
        serially by each worker with multiple iterations over data, if required.
      * solves normal equations at the worker level in the parallel case

This specialised tool matches closely BETA needs and will be used for BETA initially (at least until we converge
on the best approach to do bandpass calibration). The lifetime of this tool is uncertain at present. In many
instances the code is quick and dirty, just to suit our immediate needs.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $ <MPI wrapper> cbpcalibrator -c config.in

Parallel/Distributed Execution
------------------------------

The program is distributed and used a master/worker pattern to distribute and manage work.
Unlike *cimager* and *ccalibrator*, this program manages distribution of workload 
implicitly (c.f. with the section entitled *Parallel/Distributed Execution* in the :doc:`cimager`,
where the user is responsible for explicit workload distribution). If executed in a 
parallel environment (i.e. two or more MPI ranks are available), the program will automatically
distribute beams and channels across all available workers is uniformly as possible
(i.e. it pays to have nbeam x nchan / integer_number + 1 MPI ranks available) and the master 
will be responsible for collating and storing the results. The dataset(s) will be read multiple
times. It is possible to give a separate dataset for each beam. In this case, each dataset will
be read nchan times. 


Configuration Parameters
------------------------

Parset parameters understood by cbpcalibrator are given in the following table (all
parameters must have **Cbpcalibrator** prefix, i.e. **Cbpcalibrator.dataset**). 
Although the substitution rules (e.g. see **%w** and **%n** in :doc:`ccalibrator`)
are supported, their use is not intended.

The results are stored via *calibaccess* interface (see :doc:`calibration_solutions`), however
only casa table option currently supports bandpass products. 

A number of other parameters allowing to narrow down the data selection are understood.
They are given in [:doc:`data_selection`] and should also have the **Cbpcalibrator** prefix.

+-----------------------+----------------+--------------+-------------------------------------------------+
|**Parameter**          |**Type**        |**Default**   |**Description**                                  |
+=======================+================+==============+=================================================+
|imagetype              |string          |"casa"        |Type of the image handler (determines the format |
|                       |                |              |of the images read from the disk). The default is|
|                       |                |              |to read casa format and this is the only option  |
|                       |                |              |implemented so far.                              |
+-----------------------+----------------+--------------+-------------------------------------------------+
|nAnt                   |uint            |36            |Number of antennas to solve bandpasses for. The  |
|                       |                |              |code will fail if it is requested to solve for   |
|                       |                |              |more antennas than it has the data for           |
+-----------------------+----------------+--------------+-------------------------------------------------+
|nBeam                  |uint            |1             |Number of beams to solve for. The code           |
|                       |                |              |will fail if it is requested to solve for more   |
|                       |                |              |beams than it has the data for                   |
+-----------------------+----------------+--------------+-------------------------------------------------+
|dataset                |string or       |None          |Data set file name to produce. If the parameter  |
|                       |vector<string>  |              |is given as a vector of strings, it is           |
|                       |                |              |interpreted as one dataset per beam. Therefore,  |
|                       |                |              |the length of the string should be either 1 or   |
|                       |                |              |the number of beams.                             |
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
|refantenna             |int32           |-1            |If not negative, this is assumed to be the index |
|                       |                |              |of the reference antenna. All phases in the      |
|                       |                |              |resulting bandpass are rotated so the chosen     |
|                       |                |              |antenna has zero phase for all beams and all     |
|                       |                |              |channels                                         |
+-----------------------+----------------+--------------+-------------------------------------------------+
|sources.definition     |string          |None          |Optional parameter. If defined, sky model        |
|                       |                |              |(i.e. source info given as **sources.something**)|
|                       |                |              |is read from a separate parset file (name is     |
|                       |                |              |given by this parameter). If this parameter is   |
|                       |                |              |not defined, source description should be given  |
|                       |                |              |in the main parset file. Usual substitution rules|
|                       |                |              |apply. The parameters to define sky model are    |
|                       |                |              |described in :doc:`csimulator`(with Cbpcalibrator|
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
|calibaccess            |string          |"parset"      |The destination for the calibration solution.    |
|                       |                |              |Note, the default *parset* option does not work  |
|                       |                |              |for this application. Therefore, *table* option  |
|                       |                |              |must be used and calibaccess.table.<params>      |
|                       |                |              |parameters should be defined. For more details   |
|                       |                |              |see :doc:`calibration_solutions`.                |
+-----------------------+----------------+--------------+-------------------------------------------------+


The resulting parameters are stored into a solution source (or sink to be exact) as described in :doc:`calibration_solutions`

Example
-------

::

    Cbpcalibrator.dataset                   = calibration_data.ms
    Cbpcalibrator.nAnt                      = 6
    Cbpcalibrator.nChan                     = 304
    Cbpcalibrator.nBeam                     = 9
    Cbpcalibrator.refantenna                = 1
    Cbpcalibrator.calibaccess               = table
    Cbpcalibrator.calibaccess.table.maxbeam = 9
    Cbpcalibrator.calibaccess.table.maxant  = 6
    Cbpcalibrator.calibaccess.table.maxchan = 304

    Cbpcalibrator.sources.names             =       [src1]
    Cbpcalibrator.sources.src1.components   = [cal]
    Cbpcalibrator.sources.cal.calibrator    = 1934-638

    Cbpcalibrator.gridder                   = SphFunc
    Cbpcalibrator.ncycles                   = 5

