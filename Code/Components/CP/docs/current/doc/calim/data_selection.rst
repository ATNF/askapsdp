Data Selection Documentation
=======================================

Data accessor layer provides a capability to pre-select the data based on some criteria. As all synthesis applications see 
only data returned by the accessor, it can be used to restrict **Cimager** (see :doc:`cimager`) and **Ccalibrator** 
(see :doc:`ccalibrator`) to use a given subset of the data. All synthesis applications understand the keywords given in the 
following table (must have **Ccalibrator.** or **Cimager.** prefix), i.e. **Cimager.MinUV = 100**). All these parameters 
are optional. If present, a corresponding data selection is done and only those data will appear in the iteration using accessor layer.

+----------------------+--------------+--------------+------------------------------------------------------------+
|**Parameter**         |**Type**      |**Default**   |**Description**                                             |
+======================+==============+==============+============================================================+
|Feed                  |uint32        |None          |Select a subset of the data corresponding to a particular   |
|                      |              |              |feed (should be called synthetic beam in the ASKAP          |
|                      |              |              |terminology). If defined, only data corresponding to the    |
|                      |              |              |given beam will be read from the dataset and processed by   |
|                      |              |              |either **Cimager** or **Ccalibrator**. The value is 0-based |
|                      |              |              |number of the beam.                                         |
+----------------------+--------------+--------------+------------------------------------------------------------+
|Baseline              |vector<uint32>|None          |Select a subset of the data corresponding to a particular   |
|                      |              |              |baseline. If defined, only data from the given baseline will|
|                      |              |              |be read from the dataset and processed by either **Cimager**|
|                      |              |              |or **Ccalibrator**. The vector should have exactly two      |
|                      |              |              |elements, which are treated as zero-based antenna indices.  |
+----------------------+--------------+--------------+------------------------------------------------------------+
|Channels              |vector<uint32>|None          |Select a subset of spectral channels. If defined, only a    |
|                      |              |              |slice of the visibility cube is passed to either **Cimager**|
|                      |              |              |or **Ccalibrator.** Currently should always be a 2-element  |
|                      |              |              |vector with the number of channels requested and the first  |
|                      |              |              |channel (0-based) of the slice. The optional 3rd element of |
|                      |              |              |the vector is intended for on-the-fly averaging in spectral |
|                      |              |              |channels (it is a number of adjacent spectral channels to   |
|                      |              |              |average). It is not yet implemented by the accessor,        |
|                      |              |              |although understood at the parset level.                    |
+----------------------+--------------+--------------+------------------------------------------------------------+
|SpectralWindow        |uint32        |None          |Select data from a given spectral window only. If defined,  |
|                      |              |              |only data corresponding to this spectral window will be     |
|                      |              |              |read. The value is 0-based index of the spectral window     |
+----------------------+--------------+--------------+------------------------------------------------------------+
|Polarisations         |string        |None          |Intended for accessor-based selection of polarisation. Not  |
|                      |              |              |yet implemented.                                            |
+----------------------+--------------+--------------+------------------------------------------------------------+
|TimeRange             |vector<double>|None          |Select a subset of data based on time. If defined, only data|
|                      |              |              |within the given timerange are read. Time is defined in UTC |
|                      |              |              |seconds since MJD 0.                                        |
+----------------------+--------------+--------------+------------------------------------------------------------+
|CorrelationType       |string        |None          |Select certain type of correlations. Allowed values are:    |
|                      |              |              |**auto**, **cross** and **all**. Defining this parameter    |
|                      |              |              |allows to select, e.g. just cross-correlations.             |
+----------------------+--------------+--------------+------------------------------------------------------------+
|ScanNumber            |uint32        |None          |Select a single scan number. If defined, only data          |
|                      |              |              |corresponding to the given scan number will be read from    |
|                      |              |              |the dataset. The value is 0-based.                          |
+----------------------+--------------+--------------+------------------------------------------------------------+
|MinUV                 |double        |None          |Select data based on the uv-distance (in metres). Only      |
|                      |              |              |visibility points corresponding to uv-distance greater than |
|                      |              |              |the given value (defined in metres) are read from the       |
|                      |              |              |measurement set. Note, the selection is done without taking |
|                      |              |              |frequency information into account.                         |
+----------------------+--------------+--------------+------------------------------------------------------------+
|MaxUV                 |double        |None          |Select data based on the uv-distance (in metres). Only      |
|                      |              |              |visibility points corresponding to uv-distance smaller than |
|                      |              |              |the given value (defined in metres) are read from the       |
|                      |              |              |measurement set. Note, the selection is done without taking |
|                      |              |              |frequency information into account.                         |
+----------------------+--------------+--------------+------------------------------------------------------------+

