cimager
=======

This page provides some details about the cimager program. The purpose of this
software is to perform imaging in a parallel/distributed environment or on a single
computer system. The software leverages MPI, however can be run on a simple laptop
or a large supercomputer.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing the
configuration parameters described in the next section. ::
 
   $ <MPI wrapper> cimager -c config.in

Parallel/Distributed Execution
------------------------------

The program is distributed and used a master/worker pattern to distribute and manage work.
The program requires at least to processes to execute, and failure to either execute
*cimager* as an MPI process or specifying only one MPI process will result in an error.

On the Cray XC30 platform executing with the MPI wrapper takes the form::

    $ aprun -n 305 -N 20 cimager -c config.in

The *-n* and *-N* parameters to the *aprun* application launcher specify 305 MPI processes
will be used (304 workers and one master) and each node will host 20 MPI processes. This
job then requires 16 compute nodes.

The *cimager* has no implicit parallelism, rather the configuration parameters allow the
user to specify what subset of the total task each worker will carry out.

**Example 1:**
Suppose you have a single measurement set with 304 spectral channels, using 304 workers
(as described above) the following configuration parameter can be used to restrict each
worker to a single spectral channel::

    Cimager.Channels    = [1, %w]

Upon execution, the %w is replaced with the worker ID, a number from 0 to 303 in the case
of 304 workers.

**Example 2:**
If rather than having a single measurement set, you had 304 measurement sets each
containing a single spectral channel. In this case the measurement sets are named
*coarse_chan_0.ms* through to *coarse_chan_303.ms*. The following configuration parameter
results in each worker process processing a single measurement set::

    Cimager.dataset     = coarse_chan_%w.ms


Configuration Parameters
------------------------

Parset parameters understood by imager are given in the following table (all parameters
must have **Cimager** prefix, i.e. **Cimager.dataset**). For a number of parameters
certain keywords are substituted, i.e. **%w** is replaced by the worker number (rank-1, if
there is only one pool of workers) and **%n** by the number of nodes in the parallel case.
In the serial case, these special strings are substituted by 0 and 1, respectively. This
substitution allows to reuse the same parameter file on all nodes of the cluster if the
difference between jobs assigned to individual nodes can be coded by using these keywords
(e.g. using specially crafted file names). Note, if there is more than 1 group of workers
(e.g. parallel calculation of Taylor terms), %w index spans the workers in one group
rather than the global pool of workers. This is done to allow the same file name to be
used for corresponding worker in different groups (i.e. all Taylor terms are build from
the same file). If a parameter supports substitution, it is clearly stated in the
description. 

A number of other parameters allowing to narrow down the data selection are understood.
They are given in a separate table (see :doc:`data_selection`) and should also have the
**Cimager** prefix.
 
+--------------------------+------------------+--------------+----------------------------------------------------+
|**Parameter**             |**Type**          |**Default**   |**Description**                                     |
+==========================+==================+==============+====================================================+
|imagetype                 |string            |"casa"        |Type of the image handler (determines the format of |
|                          |                  |              |the images, both which are written to or read from  |
|                          |                  |              |the disk). The default is to create casa images and |
|                          |                  |              |this is the only option implemented so far.         |
+--------------------------+------------------+--------------+----------------------------------------------------+
|dataset                   |string or         |None          |Data set file name to produce. Usual substitution   |
|                          |vector<string>    |              |rules apply if the parameter is a single string. If |
|                          |                  |              |the parameter is given as a vector of strings all   |
|                          |                  |              |measurement sets given by this vector are           |
|                          |                  |              |effectively concatenated together on-the-fly in the |
|                          |                  |              |serial case. In the parallel case, the size of the  |
|                          |                  |              |vector is required to be either 1 or the number of  |
|                          |                  |              |nodes - 1, and therefore there is one measurement   |
|                          |                  |              |set per worker node.                                |
+--------------------------+------------------+--------------+----------------------------------------------------+
|nworkergroups             |int               |1             |Number of worker groups. This option can only be    |
|                          |                  |              |used in the parallel mode. If it is greater than 1, |
|                          |                  |              |the model parameters are distributed (as evenly as  |
|                          |                  |              |possible) between the given number of groups of     |
|                          |                  |              |workers (e.g. if one calculates a Taylor term       |
|                          |                  |              |expansion of the order of 1 for one image, setting  |
|                          |                  |              |this parameter to 3 will allow parallel computation |
|                          |                  |              |of the Taylor terms for this image). This is on top |
|                          |                  |              |of the normal parallelism within the group (the %w  |
|                          |                  |              |index spans from 0 to the number of workers per     |
|                          |                  |              |group - 1). Essentially, this option allows to throw|
|                          |                  |              |several workers on the same problem if the model    |
|                          |                  |              |allows partitioning.Taylor terms, faceting and      |
|                          |                  |              |multiple images in the model are the typical use    |
|                          |                  |              |cases.                                              |
+--------------------------+------------------+--------------+----------------------------------------------------+
|datacolumn                |string            |"DATA"        |The name of the data column in the measurement set  |
|                          |                  |              |which will be the source of visibilities.This can be|
|                          |                  |              |useful to process real telescope data which were    |
|                          |                  |              |passed through *casapy* at some stage (e.g. to image|
|                          |                  |              |calibrated data which are stored in the             |
|                          |                  |              |*CORRECTED_DATA* column). In the measurement set    |
|                          |                  |              |convention, the *DATA* column which is used by      |
|                          |                  |              |default contains raw uncalibrated data as received  |
|                          |                  |              |directly from the telescope. Calibration tasks in   |
|                          |                  |              |*casapy* make a copy when calibration is applied    |
|                          |                  |              |creating a new data column.                         |
+--------------------------+------------------+--------------+----------------------------------------------------+
|sphfuncforpsf             |bool              |false         |If true, the default spheroidal function gridder is |
|                          |                  |              |used to compute PSF regardless of the gridder       |
|                          |                  |              |selected for model degridding and residual          |
|                          |                  |              |gridding. This has a potential to produce better    |
|                          |                  |              |behaving PSF by taking out two major factors of     |
|                          |                  |              |position dependence. Note, this doesn't make the PSF|
|                          |                  |              |correct or otherwise,it is just a different         |
|                          |                  |              |approximation                                       |
+--------------------------+------------------+--------------+----------------------------------------------------+
|calibrate                 |bool              |false         |If true, calibration of visibilities will be        |
|                          |                  |              |performed before imaging. See                       |
|                          |                  |              |:doc:`calibration_solutions` for details on         |
|                          |                  |              |calibration parameters used during this application |
|                          |                  |              |process.                                            |
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
|gainsfile                 |string            |""            |This is an obsolete parameter, which is still       |
|                          |                  |              |supported for backwards compatibility defining the  |
|                          |                  |              |file with antenna gains (a parset format, keywords  |
|                          |                  |              |look like **gain.g11.0**, where g11 or g22 in the   |
|                          |                  |              |middle correspond to different polarisations and the|
|                          |                  |              |trailing number is the zero-based antenna           |
|                          |                  |              |number. The default value (empty string) means no   |
|                          |                  |              |gain correction is performed. The gain file format  |
|                          |                  |              |is the same as produced by Ccalibrator.             |
+--------------------------+------------------+--------------+----------------------------------------------------+
|restore                   |bool              |false         |If true, the image will be restored (by convolving  |
|                          |                  |              |with the given 2D gaussian). This is an additional  |
|                          |                  |              |step to normal imaging, which, by default, ends with|
|                          |                  |              |just a model image. The restored image is written   |
|                          |                  |              |into a separate image file (with the **.restore**   |
|                          |                  |              |suffix). The convolution is done with the restore   |
|                          |                  |              |solver (see also :doc:`solver`) which reuses the    |
|                          |                  |              |same parameters used to setup the image solver (and |
|                          |                  |              |therefore ensuring the same preconditioning is      |
|                          |                  |              |done). The only additional parameter of the restore |
|                          |                  |              |solver is the shape of the gaussian representing    |
|                          |                  |              |clean beam (or flag to determine the shape). It is  |
|                          |                  |              |given by the **restore.beam** parameter, which must |
|                          |                  |              |be present if **restore** is set to True            |
+--------------------------+------------------+--------------+----------------------------------------------------+
|restore.beam              |vector<string>    |None          |Either a single word *fit* or a quantity string     |
|                          |                  |              |describing the shape of the clean beam (to convolve |
|                          |                  |              |the model image with). If quantity is given it must |
|                          |                  |              |have exactly 3 elements, e.g. [30arcsec, 10arcsec,  |
|                          |                  |              |40deg]. Otherwise an exception is thrown. This      |
|                          |                  |              |parameter is only used if *restore* is set to       |
|                          |                  |              |True. If restore.beam=fit, the code will fit a 2D   |
|                          |                  |              |gaussian into PSF image (first encountered if       |
|                          |                  |              |multiple images are solved for) and use this fit    |
|                          |                  |              |results.                                            |
+--------------------------+------------------+--------------+----------------------------------------------------+
|restore.beam.cutoff       |double            |0.05          |Cutoff for the support search prior to beam         |
|                          |                  |              |fitting. This parameter is only used if             |
|                          |                  |              |*restore.beam=fit*. The code does fitting on a      |
|                          |                  |              |limited support (to speed things up and to avoid    |
|                          |                  |              |sidelobes influencing the fit). The extent of this  |
|                          |                  |              |support is controlled by this parameter representing|
|                          |                  |              |the level of the PSF which should be included into  |
|                          |                  |              |support. This value should be above the first       |
|                          |                  |              |sidelobe level for meaningful results.              |
+--------------------------+------------------+--------------+----------------------------------------------------+
|restore.equalise          |bool              |false         |If true, the final residual is multiplied by the    |
|                          |                  |              |square root of the truncated normalised weight      |
|                          |                  |              |(i.e. additional weight described by Sault et       |
|                          |                  |              |al. (1996), which gives a flat noise). Note, that   |
|                          |                  |              |the source flux densities are likely to have        |
|                          |                  |              |position-dependent errors if this option is used    |
|                          |                  |              |because not all flux is recovered during the clean  |
|                          |                  |              |process. However, the images look aesthetically     |
|                          |                  |              |pleasing with this option.                          |
+--------------------------+------------------+--------------+----------------------------------------------------+
|Images.xxx                |various           |              |A number of parameters given in this form define the|
|                          |                  |              |images one wants to produce (shapes, positions,     |
|                          |                  |              |etc). The details are given in a separate section   |
|                          |                  |              |(see below)                                         |
+--------------------------+------------------+--------------+----------------------------------------------------+
|memorybuffers             |bool              |false         |The accessor subsystem provides a buffer mechanism  |
|                          |                  |              |to work with scratch information.  If this flag is  |
|                          |                  |              |set to false (default), these buffers will be kept  |
|                          |                  |              |on disk (in a subtable of the dataset called        |
|                          |                  |              |*BUFFERS*). If this flag is set to true, scratch    |
|                          |                  |              |buffers are kept in memory and the dataset is opened|
|                          |                  |              |for read only. The imager code had been converted at|
|                          |                  |              |some stage to process the data without using scratch|
|                          |                  |              |buffers. Therefore, the status of this flag does not|
|                          |                  |              |affect the performance or memory usage. However, it |
|                          |                  |              |seems a good idea to always set it to *true* to     |
|                          |                  |              |ensure that the dataset given by the *dataset*      |
|                          |                  |              |keyword is always opened for read-only              |
+--------------------------+------------------+--------------+----------------------------------------------------+
|nUVWMachines              |int32             |1             |Size of uvw-machines cache. uvw-machines are used to|
|                          |                  |              |convert uvw from a given phase centre to a common   |
|                          |                  |              |tangent point. To reduce the cost to set the machine|
|                          |                  |              |up (calculation of the transformation matrix), a    |
|                          |                  |              |number of these machines is cached. The key to the  |
|                          |                  |              |cache is a pair of two directions: the current phase|
|                          |                  |              |centre and the tangent centre. If the required pair |
|                          |                  |              |is within the tolerances of that used to setup one  |
|                          |                  |              |of the machines in the cache, this machine is       |
|                          |                  |              |reused. If none of the cache items matches the least|
|                          |                  |              |accessed one is replaced by the new machine which is|
|                          |                  |              |set up with the new pair of directions. The code    |
|                          |                  |              |would work faster if this parameter is set to the   |
|                          |                  |              |number of phase centres encountered during          |
|                          |                  |              |imaging. In non-faceting case, the optimal setting  |
|                          |                  |              |would be the number of synthetic beams times the    |
|                          |                  |              |number of fields. For faceting (btw, the performance|
|                          |                  |              |gain is quite significant in this case), it should  |
|                          |                  |              |be further multiplied by the number of              |
|                          |                  |              |facets. Direction tolerances are given as a separate|
|                          |                  |              |parameter.                                          |
+--------------------------+------------------+--------------+----------------------------------------------------+
|uvwMachineDirTolerance    |quantity string   |"1e-6rad"     |Direction tolerance for the management of the       |
|                          |                  |              |uvw-machine cache (see *nUVWMachines* for           |
|                          |                  |              |details). The value should be an angular            |
|                          |                  |              |quantity. The default value corresponds roughly to  |
|                          |                  |              |0.2 arcsec and seems sufficient for all practical   |
|                          |                  |              |applications within the scope of ASKAPsoft.         |
+--------------------------+------------------+--------------+----------------------------------------------------+
|gridder                   |string            |None          |Name of the gridder, further parameters are given by|
|                          |                  |              |*gridder.something*. See :doc:`gridder` for details.|
|                          |                  |              |                                                    |
+--------------------------+------------------+--------------+----------------------------------------------------+
|rankstoringcf             |int               |1             |In the parallel mode, only this rank will attempt to|
|                          |                  |              |export convolution functions if this operation is   |
|                          |                  |              |requested (see *tablename* option in the            |
|                          |                  |              |:doc:`gridder`) This option is ignored in the serial|
|                          |                  |              |mode.                                               |
+--------------------------+------------------+--------------+----------------------------------------------------+
|visweights                |string            |""            |If this parameter is set to "MFS" gridders are setup|
|                          |                  |              |to grid/degrid with the weight required for         |
|                          |                  |              |multi-frequency synthesis. At the moment, this      |
|                          |                  |              |parameter is decoupled from the image setup, which  |
|                          |                  |              |has to be done separately in a consistent way to use|
|                          |                  |              |MSMFS (*nterms* should be set to something greater  |
|                          |                  |              |than 1).                                            |
+--------------------------+------------------+--------------+----------------------------------------------------+
|visweights.MFS.reffreq    |double            |1.405e9       |Reference frequency in Hz for MFS processing (see   |
|                          |                  |              |above)                                              |
+--------------------------+------------------+--------------+----------------------------------------------------+
|solver                    |string            |None          |Name of the solver, further parameters are given by |
|                          |                  |              |*solver.something*. See :doc:`solver` for details   |
|                          |                  |              |                                                    |
+--------------------------+------------------+--------------+----------------------------------------------------+
|thershold.xxx             |various           |              |Thresholds for the minor and major cycle (cycle     |
|                          |                  |              |termination criterion), see :doc:`solver` for       |
|                          |                  |              |details.                                            |
+--------------------------+------------------+--------------+----------------------------------------------------+
|preconditioner.xxx        |various           |              |Preconditioners applied to the normal equations     |
|                          |                  |              |before the solver is called, see :doc:`solver` for  |
|                          |                  |              |details.                                            |
+--------------------------+------------------+--------------+----------------------------------------------------+
|ncycles                   |int32             |0             |Number of major cycles (and iterations over the     |
|                          |                  |              |dataset)                                            |
+--------------------------+------------------+--------------+----------------------------------------------------+
|sensitivityimage          |bool              |true          |If true, an image with theoretical sensitivity will |
|                          |                  |              |be created in addition to weights image             |
+--------------------------+------------------+--------------+----------------------------------------------------+
|sensitivityimage.cutoff   |float             |0.01          |Desired cutoff in the sensitivity image             |
+--------------------------+------------------+--------------+----------------------------------------------------+
|freqframe                 |string            |topo          |Frequency frame to work in (the frame is converted  |
|                          |                  |              |when the dataset is read). Either lsrk or topo is   |
|                          |                  |              |supported.                                          |
+--------------------------+------------------+--------------+----------------------------------------------------+


Parameters of images
````````````````````

This section describes parameters used to define images, i.e. what area of the sky one wants to image and how.
All parameters given in the following table have **Cimager.Images* prefix**, e.g. Cimager.Images.reuse = false

+--------------------------+----------------+-----------------------+----------------------------------------------+
|**Parameter**             |**Type**        |**Default**            |**Description**                               |
+==========================+================+=======================+==============================================+
|reuse                     |bool            |false                  |If true, the model images will be read from   |
|                          |                |                       |the disk (from the image files they are       |
|                          |                |                       |normally written to according to the parset)  |
|                          |                |                       |before the first major cycle. If false (the   |
|                          |                |                       |default), a new empty model image will be     |
|                          |                |                       |initialised for every image solved            |
|                          |                |                       |for. Setting this parameter to true allows to |
|                          |                |                       |continue cleaning the same image if more major|
|                          |                |                       |cycles are required after inspection of the   |
|                          |                |                       |image. Note, there is little cross check that |
|                          |                |                       |the image given as an input is actually a     |
|                          |                |                       |result of the previous run of cimager with the|
|                          |                |                       |same Image parameters. So the user is         |
|                          |                |                       |responsible to ensure that the projection,    |
|                          |                |                       |shape, etc matches.                           |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|shape                     |vector<int>     |None                   |Optional parameter to define the default shape|
|                          |                |                       |for all images. If an individual *shape*      |
|                          |                |                       |parameter is specified separately for one of  |
|                          |                |                       |the images, this default value of the shape is|
|                          |                |                       |overridden. Individual *shape* parameters (see|
|                          |                |                       |below) must be given for all images if this   |
|                          |                |                       |parameter is not defined. Must be a           |
|                          |                |                       |two-element vector.                           |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|cellsize                  |vector<string>  |None                   |Optional parameter to define the default pixel|
|                          |                |                       |(or cell) size for all images. If an          |
|                          |                |                       |individual *cellsize* parameter is specified  |
|                          |                |                       |separately for one of the images, this default|
|                          |                |                       |value is overridden. Individual *cellsize*    |
|                          |                |                       |parameters (see below) must be given for all  |
|                          |                |                       |images, if this parameter is omitted. If      |
|                          |                |                       |defined, a 2-element quantity string vector is|
|                          |                |                       |expected, e.g. [6.0arcsec, 6.0arcsec]         |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|writeAtMajorCycle         |bool            |false                  |If true, the current images are written to    |
|                          |                |                       |disk after each major cycle (*.cycle* suffix  |
|                          |                |                       |is added to the name to reflect which major   |
|                          |                |                       |cycle the image corresponds to). By default,  |
|                          |                |                       |the images are only written after *ncycles*   |
|                          |                |                       |major cycles are completed.                   |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|Names                     |vector<string>  |None                   |List of image names which this imager will    |
|                          |                |                       |produce. If more than one image is given, a   |
|                          |                |                       |superposition is assumed (i.e. visibilities   |
|                          |                |                       |are fitted with a combined effect of two      |
|                          |                |                       |images; two measurement equations are simply  |
|                          |                |                       |added). Parameters of each image defined in   |
|                          |                |                       |this list must be given in the same parset    |
|                          |                |                       |using *ImageName.something* keywords (with    |
|                          |                |                       |usual prefix). Note, all image names must     |
|                          |                |                       |start with word *image* (this is how          |
|                          |                |                       |parameters representing images are            |
|                          |                |                       |distinguished from other type of free         |
|                          |                |                       |parameters in ASKAPsoft), otherwise an        |
|                          |                |                       |exception is thrown. Example of valid names   |
|                          |                |                       |are: *image.10uJy*, *image*, *imagecena*      |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.nchan         |int32           |None                   |Number of spectral planes in the image cube to|
|                          |                |                       |produce. Set it to 1 if just a 2D image is    |
|                          |                |                       |required                                      |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.frequency     |vector<double>  |None                   |Frequencies in Hz of the first and the last   |
|                          |                |                       |spectral channels to produce in the cube. The |
|                          |                |                       |range is binned into *nchan* channels and the |
|                          |                |                       |data are gridded (with MFS) into a nearest    |
|                          |                |                       |image channel (therefore, the number of image |
|                          |                |                       |channels given by the *nchan* keyword may be  |
|                          |                |                       |less than the number of spectral channels in  |
|                          |                |                       |the data. If *nchan* is 1 all data are MFS'ed |
|                          |                |                       |into a single image (however the image will   |
|                          |                |                       |have a degenerate spectral axis with the      |
|                          |                |                       |frequency defined by the average of the first |
|                          |                |                       |and the last element of this vector; it is    |
|                          |                |                       |practical to make both elements identical,    |
|                          |                |                       |when *nchan* is 1). The vector should contain |
|                          |                |                       |2 elements at all times, otherwise an         |
|                          |                |                       |exception is thrown                           |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.direction     |vector<string>  |None                   |Direction to the centre of the required image |
|                          |                |                       |(or tangent point for facets). This vector    |
|                          |                |                       |should contain a 3-element direction quantity |
|                          |                |                       |containing right ascension, declination and   |
|                          |                |                       |epoch, e.g. [12h30m00.00, -45.00.00.00,       |
|                          |                |                       |J2000]. Note that a casa style of declination |
|                          |                |                       |delimiters (dots rather than colons) is       |
|                          |                |                       |essential. Only *J2000* directions are        |
|                          |                |                       |currently supported.                          |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.tangent       |vector<string>  |""                     |Direction to the user-defined tangent point,  |
|                          |                |                       |if different from the centre of the           |
|                          |                |                       |image. This vector should contain a 3-element |
|                          |                |                       |direction quantity containing right ascension,|
|                          |                |                       |declination and epoch, e.g. [12h30m00.00,     |
|                          |                |                       |-45.00.00.00, J2000] or be empty (in this case|
|                          |                |                       |the tangent point will be in the image        |
|                          |                |                       |centre). Note that a casa style of declination|
|                          |                |                       |delimiters (dots rather than colons) is       |
|                          |                |                       |essential. Only *J2000* directions are        |
|                          |                |                       |currently supported. This option doesn't work |
|                          |                |                       |with faceting.                                |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.ewprojection  |bool            |false                  |If true, the image will be set up with the NCP|
|                          |                |                       |or SCP projection appropriate for East-West   |
|                          |                |                       |arrays (w-term is equivalent to this          |
|                          |                |                       |coordinate transfer for East-West arrays)     |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.shape         |vector<int>     |None                   |Optional parameter if the default shape       |
|                          |                |                       |(without image name prefix) is defined. This  |
|                          |                |                       |value will override the default shape for this|
|                          |                |                       |particular image. Must be a 2-element vector. |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.cellsize      |vector<string>  |None                   |Optional parameter if the default cell size   |
|                          |                |                       |(without image name prefix) is defined. This  |
|                          |                |                       |value will override the default cell size for |
|                          |                |                       |this particular image. A two-element vector of|
|                          |                |                       |quantity strings is expected, e.g. [6.0arcsec,|
|                          |                |                       |6.0arcsec]                                    |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.nfacets       |int32           |1                      |Number of facets for the given image. If      |
|                          |                |                       |greater than one, the image centre is treated |
|                          |                |                       |as a tangent point and *nfacets* facets are   |
|                          |                |                       |created for this given image                  |
|                          |                |                       |(parameters/output model images will have     |
|                          |                |                       |names like ImageName.facet.x.y, where x and y |
|                          |                |                       |are 0-based facet indices varying from 0 to   |
|                          |                |                       |*nfacet-1*).  The facets are merged together  |
|                          |                |                       |into a single image in the restore solver     |
|                          |                |                       |(i.e. it would happen only if *restore* is    |
|                          |                |                       |true).                                        |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.polarisation  |vector<string>  |["I"]                  |Polarisation planes to be produced for the    |
|                          |                |                       |image (should have at least one). Polarisation|
|                          |                |                       |conversion is done on-the-fly, so the output  |
|                          |                |                       |polarisation frame may differ from that of the|
|                          |                |                       |dataset. An exception is thrown if there is   |
|                          |                |                       |insufficient information to obtain the        |
|                          |                |                       |requested polarisation (e.g. there are no     |
|                          |                |                       |cross-pols and full stokes cube is            |
|                          |                |                       |requested). Note, ASKAPsoft uses the *correct*|
|                          |                |                       |definition of stokes parameters,              |
|                          |                |                       |i.e. *I=XX+YY*, which is different from casa  |
|                          |                |                       |and miriad (which imply I=(XX+YY)/2).The code |
|                          |                |                       |parsing the value of this parameter is quite  |
|                          |                |                       |flexible and allows many ways to define stokes|
|                          |                |                       |axis, e.g. ["XX YY"] or ["XX","YY"] or "XX,YY"|
|                          |                |                       |are all acceptable                            |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.nterms        |int32           |1                      |Number of Taylor terms for the given image. If|
|                          |                |                       |greater than one, a given number of Taylor    |
|                          |                |                       |terms is generated for the given image which  |
|                          |                |                       |are named ImageName.taylor.x, where x is the  |
|                          |                |                       |0-based Taylor order (note, it can be combined|
|                          |                |                       |with faceting causing the names to be more    |
|                          |                |                       |complex). This name substitution happens      |
|                          |                |                       |behind the scene (as for faceting) and a      |
|                          |                |                       |number of images (representing Taylor terms)  |
|                          |                |                       |is created instead of a single one. This      |
|                          |                |                       |option should be used in conjunction with     |
|                          |                |                       |*visweights* (see above) to utilize           |
|                          |                |                       |multi-scale multi-frequency algorithm. With   |
|                          |                |                       |*visweights="MFS"* the code recognizes        |
|                          |                |                       |different Taylor terms (using _taylor.x_ name |
|                          |                |                       |suffix) and applies the appropriate           |
|                          |                |                       |order-dependent weight.                       |
+--------------------------+----------------+-----------------------+----------------------------------------------+
|*ImageName*.facetstep     |int32           |min(shape(0),shape(1)) |Offset in tangent plane pixels between facet  |
|                          |                |                       |centres (assumed the same for both            |
|                          |                |                       |dimensions).  The default value is the image  |
|                          |                |                       |size, which means no overlap between facets   |
|                          |                |                       |(no overlap on the shortest axis for          |
|                          |                |                       |rectangular images). Overlap may be required  |
|                          |                |                       |to achieve a reasonable dynamic range with    |
|                          |                |                       |faceting (aliasing from the sources located   |
|                          |                |                       |beyond the facet edge). The alternative way to|
|                          |                |                       |address the same problem is the *padding*     |
|                          |                |                       |option of the gridder (see :doc:`gridder` for |
|                          |                |                       |details).                                     |
+--------------------------+----------------+-----------------------+----------------------------------------------+


Example
-------

.. code-block:: bash

    #
    # Input measurement set
    #
    Cimager.dataset                                 = 10uJy_stdtest.ms

    #
    # Define the image(s) to write
    #
    Cimager.Images.Names                            = [image.i.10uJy_clean_stdtest]
    Cimager.Images.shape                            = [2048,2048]
    Cimager.Images.cellsize                         = [6.0arcsec, 6.0arcsec]
    Cimager.Images.image.i.10uJy_clean_stdtest.frequency    = [1.420e9,1.420e9]
    Cimager.Images.image.i.10uJy_clean_stdtest.nchan        = 1
    Cimager.Images.image.i.10uJy_clean_stdtest.direction    = [12h30m00.00, -45.00.00.00, J2000]

    #
    # Use a multiscale Clean solver
    #
    Cimager.solver                                  = Clean
    Cimager.solver.Clean.algorithm                  = MultiScale
    Cimager.solver.Clean.scales                     = [0, 3, 10, 30]
    Cimager.solver.Clean.niter                      = 10000
    Cimager.solver.Clean.gain                       = 0.1
    Cimager.solver.Clean.tolerance                  = 0.1
    Cimager.solver.Clean.verbose                    = True

    Cimager.threshold.minorcycle                    = [0.27mJy, 10%]
    Cimager.threshold.majorcycle                    = 0.3mJy

    Cimager.ncycles                                 = 10

    #
    # Restore the image at the end
    #
    Cimager.restore                                 = True
    Cimager.restore.beam                            = [30arcsec, 30arcsec, 0deg]

    #
    # Use preconditioning for deconvolution
    #
    Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
    Cimager.preconditioner.Wiener.noisepower        = 100.0
    Cimager.preconditioner.GaussianTaper            = [20arcsec, 20arcsec, 0deg]

