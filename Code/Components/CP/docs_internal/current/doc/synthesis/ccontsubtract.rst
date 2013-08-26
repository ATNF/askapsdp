ccontsubtract Documentation
===========================

This page provides some details about ccontsubtract application located in: $ASKA_ROOT/Code/Components/Synthesis/synthesis/current/apps

The purpose of this software is to subtract continuum in a parallel/distributed environment or on a single computer system. The software leverages MPI, however can be run on a simple laptop or a large supercomputer.

Parset parameters understood by ccontsubtract are given in the following table (all parameters must have *CContSubtract* prefix, i.e. *CContSubtract.dataset*). For a number of parameters certain keywords are substituted, i.e. *%w* is replaced by the rank and *%n* by the number of nodes in the parallel case. In the serial case, these special strings are substituted by 0 and 1, respectively. This substitution allows to reuse the same parameter file on all nodes of the cluster if the difference between jobs assigned to 
individual nodes can be coded by using these keywords (e.g. using specially crafted file names). If a parameter supports substitution, it is clearly stated in the description. 

A number of other parameters allowing to narrow down the data selection are understood. They are given in the "Data Selection" documentation  and should also have the *CContSubtract* prefix.


+------------------------+------------+------------+----------------------------------------------------------+
|*Parameter*             |*Type*      |*Default*   |*Description*                                             |
+========================+============+============+==========================================================+
|imagetype               |string      |"casa"      |Type of the image handler (determines the format of the   |
|                        |            |            |images read from the disk). The default is to read casa   |
|                        |            |            |images and this is the only option implemented so far.    |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|dataset                 |string      |None        |Data set file name to work with (visibility data are      |
|                        |            |            |overwritten with the subtraction result. Usual            |
|                        |            |            |substitution rules apply.                                 |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|datacolumn              |string      |"DATA"      |The name of the data column in the measurement set which  |
|                        |            |            |will be the source of visibilities and which will be      |
|                        |            |            |updated. This can be useful to process real telescope data|
|                        |            |            |which were passed through *casapy* at some stage (e.g. to |
|                        |            |            |work with calibrated data which are stored in the         |
|                        |            |            |*CORRECTED_DATA* column). In the measurement set          |
|                        |            |            |convention, the *DATA* column which is used by default    |
|                        |            |            |contains raw uncalibrated data as received directly from  |
|                        |            |            |the telescope.  Calibration tasks in *casapy* make a copy |
|                        |            |            |when calibration is applied creating a new data column.   |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|sources.definition      |string      |None        |Optional parameter.  If defined, sky model (i.e. source   |
|                        |            |            |info given as *sources.something*) is read from a separate|
|                        |            |            |parset file (name is given by this parameter).If this     |
|                        |            |            |parameter is not defined, source description should be    |
|                        |            |            |given in the main parset file. Usual substitution rules   |
|                        |            |            |apply. The parameters used to define sky model are        |
|                        |            |            |described in the [[CsimulatorDocumentation                |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|gridder                 |string      |None        |Name of the gridder, further parameters are given by      |
|                        |            |            |*gridder.something*. See                                  |
|                        |            |            |[[AS02_CalibrationAndImagingGridderDocumentation          |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|visweights              |string      |""          |If this parameter is set to "MFS" gridders are setup to   |
|                        |            |            |degrid with the weight required for the models given as   |
|                        |            |            |Taylor series (i.e. multi-frequency synthesis models). At |
|                        |            |            |the moment, this parameter is decoupled from the setup of |
|                        |            |            |the model parameters.The user has to set it separately and|
|                        |            |            |in a consistent way with the model setup (the *nterms*    |
|                        |            |            |parameter in the model definition (see the                |
|                        |            |            |Csimulator documentation                                  |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|visweights.MFS.reffreq  |double      |1.405e9     |Reference frequency in Hz for MFS-model simulation (see   |
|                        |            |            |above)                                                    |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|modelReadByMaster       |bool        |true        |This parameter has effect in the parallel case only (can  |
|                        |            |            |be set to anything in the serial case without affecting   |
|                        |            |            |the result).                                              |
|                        |            |            |                                                          |
|                        |            |            |If true, the sky model is read by the master and is then  |
|                        |            |            |distributed to all workers.                               |
|                        |            |            |                                                          |
|                        |            |            |If false, each worker reads the model, which should be    |
|                        |            |            |accessible from the worker nodes. This approach cuts down |
|                        |            |            |communication when the model is too big. Workers can also |
|                        |            |            |use individual models with the help of the substitution   |
|                        |            |            |mechanism.                                                |
+------------------------+------------+------------+----------------------------------------------------------+
|nUVWMachines            |int32       |1           |Size of uvw-machines cache. uvw-machines are used to      |
|                        |            |            |convert uvw from a given phase centre to a common tangent |
|                        |            |            |point. To reduce the cost to set the machine up           |
|                        |            |            |(calculation of the transformation matrix), a number of   |
|                        |            |            |these machines is cached.                                 |
|                        |            |            |                                                          |
|                        |            |            |The key to the cache is a pair of two directions: the     |
|                        |            |            |current phase centre and the tangent centre. If the       |
|                        |            |            |required pair is within the tolerances of that used to    |
|                        |            |            |setup one of the machines in the cache, this machine is   |
|                        |            |            |reused. If none of the cache items matches the least      |
|                        |            |            |accessed one is replaced by the new machine which is set  |
|                        |            |            |up with the new pair of directions.                       |
|                        |            |            |                                                          |
|                        |            |            |The code would work faster if this parameter is set to the|
|                        |            |            |number of phase centres encountered in the dataset. In the|
|                        |            |            |non-faceting case, the optimal setting would be the number|
|                        |            |            |of synthetic beams times the number of fields. For        |
|                        |            |            |faceting (btw, the performance gain is quite significant  |
|                        |            |            |in this case), it should be further multiplied by the     |
|                        |            |            |number of facets.                                         |
|                        |            |            |                                                          |
|                        |            |            |Direction tolerances are given as a separate parameter.   |
+------------------------+------------+------------+----------------------------------------------------------+
|uvwMachineDirTolerance  |quantity    |"1e-6rad"   |Direction tolerance for the management of the uvw-machine |
|                        |string      |            |cache (see *nUVWMachines* for details). The value should  |
|                        |            |            |be an angular quantity. The default value corresponds     |
|                        |            |            |roughly to 0.2 arcsec and seems sufficient for all        |
|                        |            |            |practical applications within the scope of ASKAPsoft.     |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+
|freqframe               |string      |topo        |Frequency frame to work in (the frame is converted when   |
|                        |            |            |the dataset is read). Either lsrk or topo is supported.   |
|                        |            |            |                                                          |
+------------------------+------------+------------+----------------------------------------------------------+


Example
-------

::

    CContSubtract.dataset                                   = 10uJy_simtest.ms
    CContSubtract.sources.names                             = [10uJy]
    CContSubtract.sources.10uJy.direction                   = [12h30m00.000, -45.00.00.000, J2000]
    CContSubtract.sources.10uJy.model                       = 10uJy.model.small
    CContSubtract.sources.10uJy.components                  = [src1]
    CContSubtract.sources.src1.flux.i                       = 1.0
    CContSubtract.sources.src1.direction.ra                 = 0.00798972946469
    CContSubtract.sources.src1.direction.dec                = 0.002
    CContSubtract.sources.src2.flux.i                       = 1.0
    CContSubtract.sources.src2.direction.ra                 = -0.00511171
    CContSubtract.sources.src2.direction.dec                = 0.0
    CContSubtract.gridder                                   = AProjectWStack
    CContSubtract.gridder.AProjectWStack.wmax               = 15000
    CContSubtract.gridder.AProjectWStack.nwplanes           = 1
    CContSubtract.gridder.AProjectWStack.oversample         = 4
    CContSubtract.gridder.AProjectWStack.diameter           = 12m
    CContSubtract.gridder.AProjectWStack.blockage           = 2m
    CContSubtract.gridder.AProjectWStack.maxfeeds           = 2
    CContSubtract.gridder.AProjectWStack.maxsupport         = 1024
    CContSubtract.gridder.AProjectWStack.frequencydependent = false
