cmodel (Model Image Generator) Documentation
============================================

The *cmodel* pipeline task is responsible for extracting a local sky model (LSM)
from the global sky model (GSM) and building an image from the components and/or
images resulting from the request.

The *cmodel* program only supports the construction of continuum images, however
it does support taylor terms allowing the modeling of spectral index and curvature.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $ <MPI wrapper> cmodel -c config.in

Parallel/Distributed Execution
------------------------------

The program is distributed and used a master/worker pattern to distribute and
manage work. Each worker receives a subset of the components to image. Components are
allocated to the workers in small batches, and only when the worker is finished with
one batch is another batch allocated to it. This provides a reasonable approach to
load-balancing. Once all components have been imaged the images are reduced back to
the master and a single image file is written to disk.

The program requires at least to processes to execute, and failure to either execute
*cmodel* as an MPI process or specifying only one MPI process will result in the
following error::

    Execution requires at least 2 MPI processes (thrown in apps/cmodel.cc:66) 

On the Cray XC30 platform executing with the MPI wrapper takes the form::

    $ aprun -n 40 -N 20 cmodel -c config.in

The *-n* and *-N* parameters to the *aprun* application launcher specify 40 MPI processes
will be used (39 workers and one master) and each node will host 20 MPI processes. This
job then requires two compute nodes.

Configuration Parameters
------------------------

+----------------------+------------+-----------------------+---------------------------------------------+
|*Parameter*           |*Default*   |*Example*              |*Description*                                |
+======================+============+=======================+=============================================+
|Cmodel.gsm.database   |*None*      |dataservice            |Either "dataservice", "votable" or           |
|                      |            |                       |"asciitable".See below for additional related|
|                      |            |                       |options                                      |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.gsm.ref_freq   |*None*      |1.4GHz                 |The reference frequency for the base flux    |
|                      |            |                       |quantity stored in the GSM. Note: Eventually |
|                      |            |                       |this will just be obtained from the Sky Model|
|                      |            |                       |Service.                                     |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.bunit          |*None*      |Jy/pixel               |                                             |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.frequency      |*None*      |1.420GHz               |Frequency                                    |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.increment      |*None*      |304MHz                 |Bandwidth                                    |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.flux_limit     |*None*      |10uJy                  |Lower limit on flux. Only sources of equal of|
|                      |            |                       |greater flux will be imaged.                 |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.shape          |*None*      |[5120, 5120]           |Output image dimensions                      |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.cellsize       |*None*      |[5arcsec, 5arcsec]     |Cell size (angular size for each pixel)      |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.direction      |*None*      |[12h30m00.00,          |Image center. Must be J2000                  |
|                      |            |-45.00.00.00, J2000]   |                                             |
|                      |            |                       |                                             |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.stokes         |[I]         |[I,Q,U,V]              |Stokes parameters in the output image.       |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.output         |casa        |casa                   |Currently only support casa output           |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.filename       |*None*      |image_10uJy.skymodel   |Name of image file created                   |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.batchsize      |100         |100                    |Number of components to send worker when     |
|                      |            |                       |worker requests more work.                   |
+----------------------+------------+-----------------------+---------------------------------------------+
|Cmodel.nterms         |1           |1                      |Number of taylor term images to              |
|                      |            |                       |produce. Valid inputs are 1, 2 and 3.        |
+----------------------+------------+-----------------------+---------------------------------------------+


If *Cmodel.gsm.database* is set to *dataservice* then the *Sky Model Data Service*
is used as the global sky model source. In this case the following three options
are used.

+--------------------------+---------------+------------------+-------------------------------------+
|*Parameter*               |*Default*      |*Example*         |*Description*                        |
+==========================+===============+==================+=====================================+
|Cmodel.gsm.locator_host   |*None*         |localhost         |Host or IP address of the ICE locator|
|                          |               |                  |service                              |
+--------------------------+---------------+------------------+-------------------------------------+
|Cmodel.gsm.locator_port   |*None*         |4061              |IP port the ICE locator service is   |
|                          |               |                  |listening on                         |
+--------------------------+---------------+------------------+-------------------------------------+
|Cmodel.gsm.service_name   |*None*         |SkyModelService   |Identity of the sky model service in |
|                          |               |                  |the ICE locator service (registry)   |
+--------------------------+---------------+------------------+-------------------------------------+


If *Cmodel.gsm.database* is set to *votable* then a VOTable is used as the global sky model source.
In this case the following option is used to specify the name of the file to read in.

+--------------------------+----------------+-----------------+-------------------------------------+
|*Parameter*               |*Default*       |*Example*        |*Description*                        |
+==========================+================+=================+=====================================+
|Cmodel.gsm.file           |*None*          |inputfile.xml    |File to read                         |
+--------------------------+----------------+-----------------+-------------------------------------+


If *Cmodel.gsm.database* is set to *asciitable* then a row/column (space separated) file is used as
the global sky model source. In this case the following option is used to specify the name of the file
to read in.

+----------------------------------------+-----------+-----------+--------------------------------------+
|*Parameter*                             |*Default*  |*Example*  |*Description*                         |
+========================================+===========+===========+======================================+
|Cmodel.tablespec.ra.col                 |*None*     |3          |Column (zero based) containing the RA |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.ra.units               |*None*     |deg        |RA units (Must confirm to degrees)    |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.dec.col                |*None*     |4          |Column (zero based) containing the    |
|                                        |           |           |Declination                           |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.dec.units              |*None*     |deg        |Declination units (Must confirm to    |
|                                        |           |           |degrees)                              |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.flux.col               |*None*     |10         |Column (zero based) containing the    |
|                                        |           |           |flux                                  |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.flux.units             |*None*     |Jy         |Flux units (Must conform to Jy)       |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.majoraxis.col          |*None*     |6          |Column (zero based) containing the    |
|                                        |           |           |major axis                            |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.majoraxis.units        |*None*     |arcsec     |Major axis units (must confirm to     |
|                                        |           |           |degrees)                              |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.minoraxis.col          |*None*     |7          |Column (zero based) containing the    |
|                                        |           |           |minor axis                            |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.minoraxis.units        |*None*     |arcsec     |Major axis units (must conform to     |
|                                        |           |           |degrees)                              |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.posangle.col           |*None*     |5          |Column (zero based) containing the    |
|                                        |           |           |position angle                        |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.posangle.units         |*None*     |rad        |Position angle units (must confirm to |
|                                        |           |           |degrees)                              |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.spectralindex.col      |*None*     |12         |Column (zero based) containing the    |
|                                        |           |           |spectral index                        |
+----------------------------------------+-----------+-----------+--------------------------------------+
|Cmodel.tablespec.spectralcurvature.col  |*None*     |13         |Column (zero based) containing the    |
|                                        |           |           |spectral curvature                    |
+----------------------------------------+-----------+-----------+--------------------------------------+


Note: Neither spectral index or curvature require units.

Configuration Example
---------------------

Example 1
~~~~~~~~~

This first example demonstrates configuration using the *Sky Model Data Service* as the global sky model source.

.. code-block:: bash

    # The below specifies the GSM source is the Sky Model Service
    Cmodel.gsm.database       = dataservice
    Cmodel.gsm.locator_host   = localhost
    Cmodel.gsm.locator_port   = 4061
    Cmodel.gsm.service_name   = SkyModelService
    Cmodel.gsm.ref_freq       = 1.4GHz

    # General parameters
    Cmodel.bunit              = Jy/pixel
    Cmodel.frequency          = 1.420GHz
    Cmodel.increment          = 304MHz
    Cmodel.flux_limit         = 10uJy
    Cmodel.shape              = [5120, 5120]
    Cmodel.cellsize           = [5arcsec, 5arcsec]
    Cmodel.direction          = [12h30m00.00, -45.00.00.00, J2000]
    Cmodel.stokes             = [I]
    Cmodel.nterms             = 3

    # Output specific parameters
    Cmodel.output             = casa
    Cmodel.filename           = image_10uJy.skymodel

Example 2
~~~~~~~~~

This second example demonstrates configuration using an output file from the VOTable
source finder as the global sky model source.


.. code-block:: bash

    # The below specifies the GSM source is a duchamp output file
    Cmodel.gsm.database       = votable
    Cmodel.gsm.file           = duchamp-fitResults.xml
    Cmodel.gsm.ref_freq       = 1.421GHz

    # General parameters
    Cmodel.bunit              = Jy/pixel
    Cmodel.frequency          = 1.420GHz
    Cmodel.increment          = 304MHz
    Cmodel.flux_limit         = 10mJy
    Cmodel.shape              = [4096, 4096]
    Cmodel.cellsize           = [5arcsec, 5arcsec]
    Cmodel.direction          = [12h30m00.00, -45.00.00.00, J2000]
    Cmodel.stokes             = [I]
    Cmodel.nterms             = 3

    # Output specific parameters
    Cmodel.output             = casa
    Cmodel.filename           = image_10mJy.skymodel
