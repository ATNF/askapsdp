csimulator
==========

The csimulator program is used to generate a measurement set from a model sky.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $ csimulator -c config.in

Configuration Parameters
------------------------

Available parameters for simulator are given in the following table (all parameters
must have **Csimulator** prefix, i.e. **Csimulator.dataset**). For a number of parameters
certain keywords are substituted, i.e. **%w** is replaced by the rank and **%n** by the
number of nodes in the parallel case. In the serial case, these special strings are
substituted by 0 and 1, respectively.

This substitution allows to reuse the same parameter file on all nodes of the cluster
if the difference between jobs assigned to individual nodes can be coded in using
these keywords (i.e. using specially crafted file names). If a parameter allows
substitution, it is clearly stated in the description. 

+----------------------+--------------+--------------+------------------------------------------------------------+
|**Parameter**         |**Type**      |**Default**   |**Description**                                             |
+======================+==============+==============+============================================================+
|imagetype             |string        |"casa"        |Type of the image handler (determines the format of the     |
|                      |              |              |images read from the disk). The default is to read casa     |
|                      |              |              |images and this is the only option implemented so far.      |
+----------------------+--------------+--------------+------------------------------------------------------------+
|dataset               |string        |None          |Data set file name to produce. Usual substitution rules     |
|                      |              |              |apply.                                                      |
+----------------------+--------------+--------------+------------------------------------------------------------+
|sources.definition    |string        |None          |Optional parameter. If defined, sky model (i.e. source info |
|                      |              |              |given as **sources.something**) is read from a separate     |
|                      |              |              |parset file (name is given by this parameter). If this      |
|                      |              |              |parameter is not defined, source description should be given|
|                      |              |              |in the main parset file. Usual substitution rules apply. The|
|                      |              |              |parameters used to define sky model are described below.    |
+----------------------+--------------+--------------+------------------------------------------------------------+
|antennas.definition   |string        |None          |Optional parameter. If defined, antenna layout              |
|                      |              |              |(**antennas.something**) is read from a separate parset file|
|                      |              |              |(name is given by this parameter). Usual substitution rules |
|                      |              |              |apply. If this parameter is not defined, antenna layout must|
|                      |              |              |be given in the main parset file. A description of these    |
|                      |              |              |parameters is given in a separate section.                  |
+----------------------+--------------+--------------+------------------------------------------------------------+
|feeds.definition      |string        |None          |Optional parameter. If defined, feed information            |
|                      |              |              |(*feeds.something*) is read from a separate parset file     |
|                      |              |              |(name is given by this parameter). If this parameter is not |
|                      |              |              |defined, the appropriate parameters must be given in the    |
|                      |              |              |main parset file (see a separate section below for          |
|                      |              |              |description). Usual substitution rules apply.               |
+----------------------+--------------+--------------+------------------------------------------------------------+
|spws.definition       |string        |None          |Optional parameter. If defined, configuration of spectral   |
|                      |              |              |windows (i.e. which frequencies are observed) is read from a|
|                      |              |              |separate parset file, which name is given by this           |
|                      |              |              |parameter. If not, the appropriate parameters               |
|                      |              |              |(*spws.something*) must be given in the main parset file    |
|                      |              |              |(see a separate section below for description). Usual       |
|                      |              |              |substitution rules apply.                                   |
+----------------------+--------------+--------------+------------------------------------------------------------+
|simulation.definition |string        |None          |Optional parameter. If defined, additional parameters of    |
|                      |              |              |simulations are read from a separate parset file (name is   |
|                      |              |              |given by this parameter). If this parameter is not defined, |
|                      |              |              |these additional parameters (listed in a separate section   |
|                      |              |              |below) must be given in the main parset file. Usual         |
|                      |              |              |substitution rules apply.                                   |
+----------------------+--------------+--------------+------------------------------------------------------------+
|observe.definition    |string        |None          |Optional parameter. If defined, description of simulated    |
|                      |              |              |observations is read from a separate parset file (name is   |
|                      |              |              |given by this parameter). If this parameter is not defined, |
|                      |              |              |these details must be given in the main parset file (see the|
|                      |              |              |description in a separate section below).                   |
+----------------------+--------------+--------------+------------------------------------------------------------+
|visweights            |string        |""            |If this parameter is set to "MFS" gridders are setup to     |
|                      |              |              |degrid with the weight required for the models given as     |
|                      |              |              |Taylor series (i.e. multi-frequency synthesis models). At   |
|                      |              |              |the moment, this parameter is decoupled from the setup of   |
|                      |              |              |the model parameters. The user has to set it separately and |
|                      |              |              |in a consistent way with the model setup (*nterms* should be|
|                      |              |              |set to something greater than 1 and there should be an      |
|                      |              |              |appropriate number of models defined).                      |
+----------------------+--------------+--------------+------------------------------------------------------------+
|visweights.MFS.reffreq|double        |1.405e9       |Reference frequency in Hz for MFS-model simulation (see     |
|                      |              |              |above)                                                      |
+----------------------+--------------+--------------+------------------------------------------------------------+
|stman.bucketsize      |int32         |32768         |Set the bucket size (in bytes) of the casa table storage    |
|                      |              |              |manager. This usually translates to the I/O size.           |
|                      |              |              |                                                            |
|                      |              |              |                                                            |
|                      |              |              |                                                            |
+----------------------+--------------+--------------+------------------------------------------------------------+
|stman.tilencorr       |int32         |4             |Set the number of correlations per tile. This affects the   |
|                      |              |              |way the table is stored on disk.                            |
+----------------------+--------------+--------------+------------------------------------------------------------+
|stman.tilenchan       |int32         |32            |Set the number of channels per tile. This affects the way   |
|                      |              |              |the table is stored on disk.                                |
+----------------------+--------------+--------------+------------------------------------------------------------+
|gridder               |string        |None          |Name of the gridder, further parameters are given by        |
|                      |              |              |*gridder.something*. See                                    |
|                      |              |              |:doc:`gridder`                                              |
+----------------------+--------------+--------------+------------------------------------------------------------+
|corrupt               |bool          |False         |if True, simulated data are corrupted by simulating         |
|                      |              |              |calibration effects. See                                    |
|                      |              |              |:doc:`calibration_solutions`                                |
+----------------------+--------------+--------------+------------------------------------------------------------+
|corrupt.gainsfile     |string        |None          |This is a deprecated parameter, which will be removed in the|
|                      |              |              |future. It used to define the name of the file with         |
|                      |              |              |gains/leakages to apply if **corrupt** is true. The external|
|                      |              |              |file had a parset format, with keywords given in the form   |
|                      |              |              |**gain.g11.0 = [0.918308,0.000000]**, where g11 is for the  |
|                      |              |              |first parallel-hand polarisation and g22 is for the         |
|                      |              |              |second. The last number is the 0-based antenna number, and  |
|                      |              |              |the value is a complex value with real and imaginary parts  |
|                      |              |              |given in a vector. Leakage parameters were given in the form|
|                      |              |              |**leakage.g12.x.y** and **leakage.g21.x.y** and were used   |
|                      |              |              |only if **corrupt.leakage** is true. Now the same           |
|                      |              |              |functionality can be done using *calibaccess.parset*, see   |
|                      |              |              |:doc:`calibration_solutions`                                |
+----------------------+--------------+--------------+------------------------------------------------------------+
|corrupt.leakage       |bool          |false         |If true, polarisation leakage is simulated. Values will be  |
|                      |              |              |taken from file referred to by gainsfile.                   |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise                 |bool          |false         |if True, noise is added to the simulated visibilities. There|
|                      |              |              |are two ways to obtain noise level. It can either be given  |
|                      |              |              |manually using **noise.variance** or **noise.rms**          |
|                      |              |              |parameters or can be calculated automatically if Tsys and   |
|                      |              |              |efficiency are given. There should be enough data for this  |
|                      |              |              |calculation, otherwise an exception is thrown.              |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise.variance        |double        |None          |variance in Jy^2 of the Gaussian noise added to visibilities|
|                      |              |              |(to every element of the cube, so the noise level should be |
|                      |              |              |appropriate for single polarisation, single spectral        |
|                      |              |              |channel). This parameter is only used if *noise* is true and|
|                      |              |              |is incompatible with any other noise-defining parameters    |
|                      |              |              |like **rms**, **Tsys** or **efficiency**                    |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise.rms             |double        |None          |rms in Jy of the Gaussian noise added to visibilities (to   |
|                      |              |              |every element of the cube, so the noise level should be     |
|                      |              |              |appropriate for single polarisation, single spectral        |
|                      |              |              |channel). This parameter is only used if *noise* is true and|
|                      |              |              |is incompatible with any other noise-defining parameters    |
|                      |              |              |like *variance*, *Tsys* or *efficiency*                     |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise.Tsys            |double        |None          |Tsys in Kelvins. This parameter should only come in pair    |
|                      |              |              |with **efficiency**. If given, neither **rms**, nor         |
|                      |              |              |**variance** should be defined. If set, the noise level is  |
|                      |              |              |estimated automatically using observation parameters.       |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise.efficiency      |double        |None          |Beam efficiency. This parameter should only come in pair    |
|                      |              |              |with **Tsys**. If given, neither **rms**, nor **variance**  |
|                      |              |              |should be defined. If set, the noise level is estimated     |
|                      |              |              |automatically using observation parameters.                 |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise.seed1           |string or     |"time"        |First seed of the random generator. Usual substitution rules|
|                      |int32         |              |apply (i.e. it is possible to have rank-dependent seed by   |
|                      |              |              |specifying "%w"). If the word "time" is given, the seed will|
|                      |              |              |be taken from the timer.                                    |
+----------------------+--------------+--------------+------------------------------------------------------------+
|noise.seed2           |string or     |"%w"          |Second seed of the random generator. Usual substitution     |
|                      |int32         |              |rules apply (i.e. it is possible to have rank-dependent seed|
|                      |              |              |by specifying "%w"). If the word "time" is given, the seed  |
|                      |              |              |will be taken from the timer.                               |
+----------------------+--------------+--------------+------------------------------------------------------------+
|modelReadByMaster     |bool          |true          |This parameter has effect in the parallel case only (can be |
|                      |              |              |set to anything in the serial case without affecting the    |
|                      |              |              |result). If true, the sky model is read by the master and is|
|                      |              |              |then distributed to all workers. If false, each worker reads|
|                      |              |              |the model, which should be accessible from the worker       |
|                      |              |              |nodes. This approach cuts down communication when the model |
|                      |              |              |is too big. Workers can also use individual models with the |
|                      |              |              |help of the substitution mechanism.                         |
+----------------------+--------------+--------------+------------------------------------------------------------+
|msWrittenByMaster     |bool          |false         |If true, the workers send data to the master which writes a |
|                      |              |              |single measurement set, otherwise each worker writes its own|
|                      |              |              |measurement set which name is either given explicitly or via|
|                      |              |              |the substitution rule. The prediction work is distributed as|
|                      |              |              |evenly as possible between all available workers (frequency |
|                      |              |              |channels are split). The option is allowed in the parallel  |
|                      |              |              |case only. The substitution has no effect when this option  |
|                      |              |              |used in most cases, and %w is replaced by -1 (note, it works|
|                      |              |              |for the random seed).                                       |
+----------------------+--------------+--------------+------------------------------------------------------------+



Parameters of the sources (or fields)
-------------------------------------

This section describes how the sky model. The parameters can be given either in the main parset file or in a separate
one pointed by *sources.definition* (see above). All parameters below have *Csimulator* prefix, if given in the main parset file.

+------------------------+--------------+--------------+---------------------------------------------------------+
|**Parameter**           |**Type**      |**Default**   |**Description**                                          |
+========================+==============+==============+=========================================================+
|sources.names           |vector<string>|None          |List of sources (or fields) to simulate. Each field is   |
|                        |              |              |definded by an image and/or a number of components. The  |
|                        |              |              |name is used to reference the details on the             |
|                        |              |              |corresponding sky model given in separate parameters (see|
|                        |              |              |below) and can be arbitrary.                             |
+------------------------+--------------+--------------+---------------------------------------------------------+
|sources.nameOfSource.xxx|              |              |Additional parameters for the source *nameOfSource*,     |
|                        |              |              |specifying either image-based model or component-based   |
|                        |              |              |model (or both). These are described below.              |
+------------------------+--------------+--------------+---------------------------------------------------------+



All following parameters have *Csimulator.sources.nameOfSource* prefix.

+---------------------------+--------------+--------------+------------------------------------------------------+
|**Parameter**              |**Type**      |**Default**   |**Description**                                       |
+===========================+==============+==============+======================================================+
|direction                  |direction     |None          |Direction to the source or field (given as direction  |
|                           |              |              |string, e.g. **[12h30m00.000, -15.00.00.000,          |
|                           |              |              |J2000]**). If the model is defined by an image, this  |
|                           |              |              |parameter should match the coordinate system in the   |
|                           |              |              |image. For components, this value is supposed to be a |
|                           |              |              |reference position, but is not used at the moment     |
|                           |              |              |(this is the current limitation of the component code |
|                           |              |              |that all components are defined with respect to the   |
|                           |              |              |phase centre and therefore are always replicated for  |
|                           |              |              |all synthetic beams).                                 |
+---------------------------+--------------+--------------+------------------------------------------------------+
|model                      |vector<string>|None          |If this parameter is defined, an image-based model is |
|                           |              |              |used for the source. The value of this parameter is   |
|                           |              |              |the file name of the image. Usual substituting rules  |
|                           |              |              |apply (see the description at the top of the          |
|                           |              |              |page). However, the substitution only makes sense if  |
|                           |              |              |workers read the model (see **modelReadByMaster**     |
|                           |              |              |parameter). If more than one model is given, it is    |
|                           |              |              |assumed that the model is given as Taylor series      |
|                           |              |              |(**nterms** parameter described below should match the|
|                           |              |              |number of models in this case) and each image         |
|                           |              |              |corresponds to the appropriate Taylor term starting   |
|                           |              |              |from 0. If only one model image is given and          |
|                           |              |              |**nterms** is not 1, the name is treated as base name |
|                           |              |              |and .taylor.x suffix is appended to each name         |
+---------------------------+--------------+--------------+------------------------------------------------------+
|nterms                     |int           |1             |Number of taylor terms in the given image-based       |
|                           |              |              |model. See the **model** keyword for the supported    |
|                           |              |              |ways to define individual Taylor terms.               |
+---------------------------+--------------+--------------+------------------------------------------------------+
|components                 |vector<string>|None          |list of components (names) to simulate for this source|
|                           |              |              |(or field). Each component defined by parameters      |
|                           |              |              |**componentName.xxx** as below (with just             |
|                           |              |              |**Csimulator.sources.nameOfSource** prefix)           |
+---------------------------+--------------+--------------+------------------------------------------------------+
|componentName.flux.i       |double        |None          |Flux of the given component                           |
+---------------------------+--------------+--------------+------------------------------------------------------+
|componentName.direction.ra |double        |None          |RA offset from the field centre for the given         |
|                           |              |              |component (in radians)                                |
+---------------------------+--------------+--------------+------------------------------------------------------+
|componentName.direction.dec|double        |None          |Dec offset from the field centre for the given        |
|                           |              |              |component (in radians)                                |
+---------------------------+--------------+--------------+------------------------------------------------------+
|componentName.shape.bmaj   |double        |None          |Required only for a gaussian component. Major axis of |
|                           |              |              |the gaussian (in radians) for this component.         |
+---------------------------+--------------+--------------+------------------------------------------------------+
|componentName.shape.bmin   |double        |None          |Required only for a gaussian component. Minor axis of |
|                           |              |              |the gaussian (in radians) for this component.         |
+---------------------------+--------------+--------------+------------------------------------------------------+
|componentName.shape.bpa    |double        |None          |Required only for a gaussian component. Position angle|
|                           |              |              |of the gaussian (in radians) for this component.      |
+---------------------------+--------------+--------------+------------------------------------------------------+



Definition of the array layout
------------------------------

This section describes how the array layout is defined. The parameters can be given either in the main parset file or in a separate
one pointed by *antennas.definition* (see above). All parameters below have *Csimulator* prefix, if given in the main parset file.

+--------------------------------+--------------+--------------+-------------------------------------------------+
|**Parameter**                   |**Type**      |**Default**   |**Description**                                  |
+================================+==============+==============+=================================================+
|antennas.telescope              |string        |None          |name of the array, e.g. ASKAP. This name is used |
|                                |              |              |to compose other parameter names (see below)     |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.names      |vector<string>|None          |List of antenna names included into array,       |
|                                |              |              |e.g. ANT1, ANT2, etc. These names are used to    |
|                                |              |              |form the parameter name to define the position of|
|                                |              |              |each antenna (in the form                        |
|                                |              |              |*antennas.nameOfArray.antennaName*, see          |
|                                |              |              |below). For useful operation should contain at   |
|                                |              |              |least 2 antennas.                                |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.mount      |string        |equatorial    |Antenna mount (must be the same for the whole    |
|                                |              |              |array). Only _equatorial_ (default) or _alt-\az_ |
|                                |              |              |mounts are allowed. Use _equatorial_ to simulate |
|                                |              |              |ASKAP's 3-axis mount (assumes perfect            |
|                                |              |              |compensation for the parallactic angle rotation) |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.diameter   |quantity      |"12m"         |Diameter of the antennas (assumed the same for   |
|                                |string        |              |the whole array)                                 |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.coordinates|string        |local         |Type of the coordinate system used to define     |
|                                |              |              |antenna position. Allowed values are *global* and|
|                                |              |              |*local*. This string is passed directly to the   |
|                                |              |              |casacore's NewMSSimulator, which is doing the    |
|                                |              |              |actual job to generate metadata. If *local*      |
|                                |              |              |(default) the antenna coordinates are treated as |
|                                |              |              |offsets from the reference location. If *global* |
|                                |              |              |they are offsets w.r.t. the Earth Centre and the |
|                                |              |              |coordinate axes are aligned with ITRF. Note, this|
|                                |              |              |is not the how we normally use the simulator     |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.scale      |float         |1.0           |Optional scaling factor for the antenna          |
|                                |              |              |layout. Default is no scaling.                   |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.antennaName|vector<float> |None          |Coordinates (in the form [x,y,z], the values are |
|                                |              |              |in metres) for antenna with name                 |
|                                |              |              |antennaName. There should be one such parameter  |
|                                |              |              |for each antenna listed in                       |
|                                |              |              |*antennas.nameOfArray.names* (parameters for     |
|                                |              |              |antennas not listed in there are simply          |
|                                |              |              |ignored). Coordinates are multiplied by the scale|
|                                |              |              |before being passed to casacore's NewMSSimulator,|
|                                |              |              |which is responsible for simulation of metadata. |
+--------------------------------+--------------+--------------+-------------------------------------------------+
|antennas.nameOfArray.location   |vector<string>|None          |Centre location for the array layout given as a  |
|                                |              |              |4-element vector with longitude, latitude,       |
|                                |              |              |altitude (all given as quantities) and reference |
|                                |              |              |frame, i.e. **[+115deg, -26deg, 192km,           |
|                                |              |              |WGS84]**. For *local* coordinates (see above),   |
|                                |              |              |this is the origin of the coordinate system where|
|                                |              |              |antenna positions are defined (axes point to the |
|                                |              |              |East, North and to the local zenith). For        |
|                                |              |              |*global* coordinates this position is used to    |
|                                |              |              |determine whether the source is visible          |
|                                |              |              |(casacore's NewMSSimulator doesn't properly      |
|                                |              |              |support VLBI-scale baselines), although          |
|                                |              |              |geocentric X,Y,Z define the antenna positions on |
|                                |              |              |the ground                                       |
+--------------------------------+--------------+--------------+-------------------------------------------------+
 


Definition of the feed configuration
------------------------------------

This section describes how the feed (strictly speaking should call it a synthetic beam) layout is defined. The
parameters can be given either in the main parset file or in a separate parset file pointed by *feeds.definition*
(see above). All parameters below have *Csimulator* prefix, if given in the main parset file.

+--------------+-------------------+------------+-------------------------------------------------------------+
|**Parameter** |**Type**           |**Default** |**Description**                                              |
+==============+===================+============+=============================================================+
|feeds.names   |vector<string>     |None        |List of beams to define (e.g. [Beam1,Beam2]), at least one   |
|              |                   |            |should be defined.  The names are used to compose the        |
|              |                   |            |parameter name (in the form *feeds.beamName*) defining       |
|              |                   |            |angular offsets from the boresight                           |
+--------------+-------------------+------------+-------------------------------------------------------------+
|feeds.mode    |string             |"perfect X  |Polarisation properties of each beam (assumed the same for   |
|              |                   |Y"          |all). Any string understood by casacore is supported. Default|
|              |                   |            |is perfect (i.e. not sensitive to circular or orthogonal     |
|              |                   |            |linear polarisation) linears.  Note, that although "perfect L|
|              |                   |            |R" is supported here, in some other places the linear        |
|              |                   |            |receptors are implicitly assumed.                            |
+--------------+-------------------+------------+-------------------------------------------------------------+
|feeds.beamName|vector<double>     |None        |Dimensionless offset of the given beam from the boresight    |
|              |                   |            |direction (given as [x,y]). Values are multiplied by         |
|              |                   |            |*feeds.spacing* before being passed to casacore's            |
|              |                   |            |NewMSSimulator, which does the actual job of simulating the  |
|              |                   |            |metadata.This also defines the units (assumed the same for   |
|              |                   |            |all beams) to get a correct angular quantity.If              |
|              |                   |            |*feeds.spacing* is not defined, the values in this parameter |
|              |                   |            |are treated as angular offsets in radians.  The offsets      |
|              |                   |            |should be defined for every beam listed in                   |
|              |                   |            |*feeds.names*. Parameters corresponding to beams which are   |
|              |                   |            |not listed in there are ignored                              |
+--------------+-------------------+------------+-------------------------------------------------------------+
|feeds.spacing |quantity string    |None        |Optional parameter. If present, it determines the dimension  |
|              |                   |            |and scaling of the beam layout (see above). If not defined,  |
|              |                   |            |all beam offsets are assumed to be in radians.               |
+--------------+-------------------+------------+-------------------------------------------------------------+



Definition of the spectral windows
----------------------------------

This section describes how the spectral windows (i.e. frequency mapping) is defined. The parameters can be given
either in the main parset file or in a separate parset file pointed by *spws.definition* (see above). All
parameters below have *Csimulator* prefix, if given in the main parset file.

+------------------+--------------+------------+--------------------------------------------------------------+
|**Parameter**     |**Type**      |**Default** |**Description**                                               |
+==================+==============+============+==============================================================+
|spws.names        |vector<string>|None        |List of names for all spectral windows. Names are used to     |
|                  |              |            |define parameters for each spectral window (in the form       |
|                  |              |            |*spws.nameOfWindow*). Spectral windows defined, but not listed|
|                  |              |            |here are ignored.                                             |
+------------------+--------------+------------+--------------------------------------------------------------+
|spws.nameOfWindow |vector<string>|None        |A 4-element vector describing the actual spectral window (or  |
|                  |              |            |correlator setup) configuration containing the number of      |
|                  |              |            |channels, frequency of the first channel (quantity), frequency|
|                  |              |            |increment (quantity) and polarisation products required (given|
|                  |              |            |as *[1, 1.420GHz, -16MHz, "XX XY YX YY"]*). The current code  |
|                  |              |            |provides enough flexibility to simulate various polarisation  |
|                  |              |            |products including mixed ones, i.e "XX RR I", as long as the  |
|                  |              |            |inputs are sufficient to make the transformation.             |
+------------------+--------------+------------+--------------------------------------------------------------+



Additional parameters of simulation
-----------------------------------

This section describes how simulations can be fine tuned. The parameters listed below can be given either in the
main parset file or in a separate parset file pointed by *simulation.definition* (see above). All parameters
below have *Csimulator* prefix, if they are defined in the main parset file.

+----------------------------+-----------------+----------+------------------------------------------------------+
|**Parameter**               |**Type**         |*Default* |**Description**                                       |
+============================+=================+==========+======================================================+
|simulation.blockage         |double           |0.0       |Fractional blocakge limit to determine whether antenna|
|                            |                 |          |is shadowed. It is passed directly to casacore's      |
|                            |                 |          |NewMSSimulator which generates the actual metadata. If|
|                            |                 |          |the antenna aperture area larger than this fraction   |
|                            |                 |          |from the total area is shadowed, all visibilities     |
|                            |                 |          |including this antenna are flagged. (*MV:*) It is     |
|                            |                 |          |probably safer to use a very small (like *1e-6* which |
|                            |                 |          |is a default in casacore, btw) value, rather than 0 to|
|                            |                 |          |achieve flagging for shadowing of any extent. Because |
|                            |                 |          |the fractional area is always non-zero. The flagging  |
|                            |                 |          |condition in casacore could be satisfied due to       |
|                            |                 |          |round-off error causing spurious flagging. However, it|
|                            |                 |          |is worth noting that we haven't seen such effect in   |
|                            |                 |          |the current simulations.                              |
+----------------------------+-----------------+----------+------------------------------------------------------+
|simulation.elevationlimit   |quantity string  |"8deg"    |Elevation limit of all antennas. If source elevation  |
|                            |                 |          |is below this value, corresponding visibilities are   |
|                            |                 |          |flagged. Note, casacore's NewSimulator calculates the |
|                            |                 |          |source elevation for the reference location           |
|                            |                 |          |only. Therefore, even if the array is sparse enough   |
|                            |                 |          |all baselines will be flagged at the same time.       |
+----------------------------+-----------------+----------+------------------------------------------------------+
|simulation.autocorrwt       |double           |0.0       |Relative weight given to autocorrelations (default    |
|                            |                 |          |value of 0.0 means the weight will be 0). Csimulator  |
|                            |                 |          |assumes the same diameter for all antennas. In this   |
|                            |                 |          |case, an equal weight of 1 will be generated for all  |
|                            |                 |          |visibilities. Auto-correlations will be assigned a    |
|                            |                 |          |weight equal to this factor. (*MV:*) I see no use in  |
|                            |                 |          |this factor given the equal diameter assumption. It   |
|                            |                 |          |should probably be set to 1.0 rather than 0.0 if, in  |
|                            |                 |          |the future, we want to do something with              |
|                            |                 |          |autocorrelations.                                     |
+----------------------------+-----------------+----------+------------------------------------------------------+
|simulation.integrationtime  |quantity string  |"10s"     |Simulated integration time of the correlator.         |
+----------------------------+-----------------+----------+------------------------------------------------------+
|simulation.usehourangles    |bool             |True      |if True, the start and stop time are interpreted as   |
|                            |                 |          |hour angles, rather than actual UT times              |
+----------------------------+-----------------+----------+------------------------------------------------------+
|simulation.referencetime    |epoch string     |None      |Reference epoch used to specify start and stop time,  |
|                            |                 |          |e.g. [2007Mar07, UTC]                                 |
+----------------------------+-----------------+----------+------------------------------------------------------+



Parameters of simulated observations
------------------------------------

This section describes how to setup parameters of the observation to be simulated. The parameters listed below can
be given either in the main parset file or in a separate parset file pointed by *observe.definition* (see above).
All parameters below have *Csimulator* prefix, if they are defined in the main parset file.

+---------------------+----------------+------------+---------------------------------------------------------------------+
|**Parameter**        |**Type**        |**Default** |**Description**                                                      |
+=====================+================+============+=====================================================================+
| observe.number      | int            | 0          | Number of scans to simulate (should be non-zero, default value would|
|                     |                |            | cause an exception). Parameters describing each scan are given by   |
|                     |                |            | *observe.scanN*, where *scanN* is zero-based number of the scan. An |
|                     |                |            | exception is thrown if such a parameter is missing for any of the   |
|                     |                |            | simulated scans.                                                    |
+---------------------+----------------+------------+---------------------------------------------------------------------+
| observe.scanN       | vector<string> | None       | Parameters for the scan N (0..number-1) specified as a 4-element    |
|                     |                |            | vector (e.g. [10uJy, Wide0, -0.0416667h, 0.0416667h]). Usual        |
|                     |                |            | substitute rules apply for the first two elements of the vector. The|
|                     |                |            | first element is the source name (see definition of sources or      |
|                     |                |            | fields), the second is the spectral window name (see definition of  |
|                     |                |            | spectral windows). The last two elements are treated as quantities  |
|                     |                |            | and represent start and stop time (or hour angle if                 |
|                     |                |            | *simulation.usehourangles* is True)                                 |
+---------------------+----------------+------------+---------------------------------------------------------------------+


Examples
--------

.. code-block:: bash

    Csimulator.dataset                              =       10uJy_stdtest.ms

    #
    # The name of the model source is 10uJy. Specify direction and model file
    #
    Csimulator.sources.names                        =       [10uJy]
    Csimulator.sources.10uJy.direction              =       [12h30m00.000, -45.00.00.000, J2000]
    Csimulator.sources.10uJy.model                  =       10uJy.model.small

    #
    # Define the antenna locations, feed locations, and spectral window definitions
    #
    Csimulator.antennas.definition                  =       definitions/ASKAP45.in
    Csimulator.feeds.definition                     =       definitions/ASKAP1feeds.in
    Csimulator.spws.definition                      =       definitions/ASKAPspws.in

    #
    # Standard settings for the simulaton step
    #
    Csimulator.simulation.blockage                  =       0.01
    Csimulator.simulation.elevationlimit            =       8deg
    Csimulator.simulation.autocorrwt                =       0.0
    Csimulator.simulation.usehourangles             =       True
    Csimulator.simulation.referencetime             =       [2007Mar07, UTC]

    #
    # Undersample in time by ~ 10 to make the processing run quickly
    #
    Csimulator.simulation.integrationtime           =       150s

    #
    # Observe source 10uJy for 12 hours with a single channel spectral window
    #

    Csimulator.observe.number                       =       1
    Csimulator.observe.scan0                        =       [10uJy, Continuum0, -6h, 6h]

    #
    # Use a gridder to apply primary beam during the W projection step.
    #
    Csimulator.gridder                              = AWProject
    Csimulator.gridder.AWProject.wmax               = 15000
    Csimulator.gridder.AWProject.nwplanes           = 129
    Csimulator.gridder.AWProject.oversample         = 8
    Csimulator.gridder.AWProject.diameter           = 12m
    Csimulator.gridder.AWProject.blockage           = 2m
    Csimulator.gridder.AWProject.maxfeeds           = 1
    Csimulator.gridder.AWProject.maxsupport         = 1024
    Csimulator.gridder.AWProject.frequencydependent = false
    Csimulator.gridder.AWProject.tablename          = AWProject.tab
