cpingest (Ingest Pipeline) Documentation
========================================

Intro and Summary
-----------------

This page provides some details about....

*WARNING* This software is still in development and should be considered volatile.

Running the program
-------------------

The following will run the ingest pipeline with two processes: 

::

    mpirun -np 2 $ASKAP_ROOT/Code/Components/CP/ingest/current/apps/cpingest.sh -c cpingest.in

although the program can be run as a single process without MPI, but with the "-s" option:

::

    $ASKAP_ROOT/Code/Components/CP/ingest/current/apps/cpingest.sh -s -c cpingest.in


Configuration Parameters
------------------------

Where there is no default, this indicates the field is required

Metadata Source
~~~~~~~~~~~~~~~

+------------------------------------------------+---------+----------+-------------------------------------------+
|*Parameter*                                     |*Type*   |*Default* |*Description*                              |
+================================================+=========+==========+===========================================+
|cp.ingest.metadata_source.ice.locator_host      |String   |None      |Host name of the ICE locator service for   |
|                                                |         |          |the TOS metadata stream                    |
|                                                |         |          |                                           |
+------------------------------------------------+---------+----------+-------------------------------------------+
|cp.ingest.metadata_source.ice.locator_port      |String   |None      |Port number (or service from /etc/services)|
|                                                |         |          |of the ICE locator service for the TOS     |
|                                                |         |          |metadata stream                            |
|                                                |         |          |                                           |
+------------------------------------------------+---------+----------+-------------------------------------------+
|cp.ingest.metadata_source.icestorm.topicmanager |String   |None      |Name of the topic manager via which the    |
|                                                |         |          |topic for the TOS metadata stream          |
|                                                |         |          |                                           |
+------------------------------------------------+---------+----------+-------------------------------------------+
|cp.ingest.metadata_source.icestorm.topic        |String   |None      |Name of the topic on which the TOS metadata|
|                                                |         |          |stream is published                        |
|                                                |         |          |                                           |
+------------------------------------------------+---------+----------+-------------------------------------------+
|cp.ingest.metadata_source.buffer_size           |Integer  |12        |Size (in messages)of the circular buffer   |
|                                                |         |          |into which the metadata is initially       |
|                                                |         |          |received. There is one metadata message    |
|                                                |         |          |received per integration cycle. The default|
|                                                |         |          |of 12, given a 5 second correlator         |
|                                                |         |          |integration cycle allows buffering of one  |
|                                                |         |          |minute worth of metadata. Message size     |
|                                                |         |          |should be in the 10's of MB at most.       |
|                                                |         |          |                                           |
+------------------------------------------------+---------+----------+-------------------------------------------+



Visibilities Source
~~~~~~~~~~~~~~~~~~~

+----------------------------------------------+-----------+-----------+------------------------------------------+
|*Parameter*                                   |*Type*     |*Default*  |*Description*                             |
+==============================================+===========+===========+==========================================+
|cp.ingest.vis_source.port                     |Integer    |None       |UDP Port number on which to listen on for |
|                                              |           |           |the visibility stream                     |
|                                              |           |           |                                          |
+----------------------------------------------+-----------+-----------+------------------------------------------+
|cp.ingest.vis_source.buffer_size              |Integer    |455544     |Size (in messages) of the circular buffer |
|                                              |           |           |into which the metadata is initially      |
|                                              |           |           |received. There is one message received   |
|                                              |           |           |per baseline, per coarse channel, per     |
|                                              |           |           |beam. One message is approximately 2KB in |
|                                              |           |           |size. The default allows for one full     |
|                                              |           |           |integration to be buffered; 666 (includes |
|                                              |           |           |auto-correlations) * 19 (304 total divided|
|                                              |           |           |by 16 ingest nodes) * 36 (maximum number  |
|                                              |           |           |of beams). This is ~900MB in size, given  |
|                                              |           |           |455544 * 2KB = 911,088KB.                 |
|                                              |           |           |                                          |
+----------------------------------------------+-----------+-----------+------------------------------------------+



Calc UVW Task
~~~~~~~~~~~~~

+----------------------------------------------+-----------+-----------+------------------------------------------+
|*Parameter*                                   |*Type*     |*Default*  |*Description*                             |
+==============================================+===========+===========+==========================================+
|cp.ingest.uvw.antennas.location               |           |           |                                          |
|                                              |           |           |                                          |
+----------------------------------------------+-----------+-----------+------------------------------------------+
|cp.ingest.uvw.antennas.names                  |           |           |                                          |
|                                              |           |           |                                          |
+----------------------------------------------+-----------+-----------+------------------------------------------+
|cp.ingest.uvw.antennas.scale                  |           |           |                                          |
|                                              |           |           |                                          |
+----------------------------------------------+-----------+-----------+------------------------------------------+
|cp.ingest.uvw.antennas.<antenna name>         |           |           |                                          |
|                                              |           |           |                                          |
+----------------------------------------------+-----------+-----------+------------------------------------------+



Calibration Task
~~~~~~~~~~~~~~~~

+---------------------------------------------------+----------+----------+---------------------------------------+
|*Parameter*                                        |*Type*    |*Default* |*Description*                          |
+===================================================+==========+==========+=======================================+
|cp.ingest.task.CalTask.params.source.type          |          |          |                                       |
|                                                   |          |          |                                       |
+---------------------------------------------------+----------+----------+---------------------------------------+
|cp.ingest.task.CalTask.params.source.locator_host  |          |          |                                       |
|                                                   |          |          |                                       |
|                                                   |          |          |                                       |
+---------------------------------------------------+----------+----------+---------------------------------------+
|cp.ingest.task.CalTask.params.source.locator_port  |          |          |                                       |
|                                                   |          |          |                                       |
|                                                   |          |          |                                       |
+---------------------------------------------------+----------+----------+---------------------------------------+
|cp.ingest.task.CalTask.params.source.service_name  |          |          |                                       |
|                                                   |          |          |                                       |
|                                                   |          |          |                                       |
+---------------------------------------------------+----------+----------+---------------------------------------+



Channel Averaging Task 
~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------------+-----------+-----------+-------------------------------------------+
|*Parameter*                                  |*Type*     |*Default*  |*Description*                              |
+=============================================+===========+===========+===========================================+
|cp.ingest.chanavg.averaging                  |Integer    |None       |Number of channels to average together.    |
|                                             |           |           |                                           |
+---------------------------------------------+-----------+-----------+-------------------------------------------+


Measurement Set Sink
~~~~~~~~~~~~~~~~~~~~

+---------------------------------------------+-----------+-----------+-------------------------------------------+
|*Parameter*                                  |*Type*     |*Default*  |*Description*                              |
+=============================================+===========+===========+===========================================+
|cp.ingest.ms_sink.filename                   |String     |None       |Name of the measurement set to write.      |
|                                             |           |           |                                           |
+---------------------------------------------+-----------+-----------+-------------------------------------------+
|cp.ingest.ms_sink.stman.bucketsize           |Integer    |104448     |Set the bucket size (in bytes) of the casa |
|                                             |           |           |table storage manager. This usually        |
|                                             |           |           |translates to the I/O size.                |
|                                             |           |           |                                           |
+---------------------------------------------+-----------+-----------+-------------------------------------------+
|cp.ingest.ms_sink.stman.tilencorr            |Integer    |4          |Set the number of correlations per         |
|                                             |           |           |tile. This affects the way the table is    |
|                                             |           |           |stored on disk.                            |
|                                             |           |           |                                           |
+---------------------------------------------+-----------+-----------+-------------------------------------------+
|cp.ingest.ms_sink.stman.tilenchan            |Integer    |1          |Set the number of channels per tile. This  |
|                                             |           |           |affects the way the table is stored on     |
|                                             |           |           |disk.                                      |
|                                             |           |           |                                           |
+---------------------------------------------+-----------+-----------+-------------------------------------------+



Configuration Parameters Example
--------------------------------

.. code-block:: bash

    #
    # Central Processor Ingest Pipeline Configuration
    #

    #
    # This defines a list of tasks to create. They will be added to the pipeline
    # in the same order as defined below. So the far left task is the first called
    # after the data is sources into the pipeline.
    #
    cp.ingest.tasklist = [CalcUVWTask, ChannelAvgTask, CalTask, MSSink]

    #
    # Merged Source
    # 
    cp.ingest.MergedSource.metadata_source.ice.locator_host         = localhost
    cp.ingest.MergedSource.metadata_source.ice.locator_port         = 4061
    cp.ingest.MergedSource.metadata_source.icestorm.topicmanager    = IceStorm/TopicManager
    cp.ingest.MergedSource.metadata_source.icestorm.topic           = tosmetadata
    cp.ingest.MergedSource.metadata_source.buffer_size              = 12
    cp.ingest.MergedSource.vis_source.port                          = 3000
    cp.ingest.MergedSource.vis_source.buffer_size                   = 459648
    # The above is 21 (baselines) * 36 (beams) * 304 (coarse channels) * 4 cycles

    #
    # Calc UVW Task
    #
    cp.ingest.task.CalcUVWTask.type                     = CalcUVWTask

    #
    # Channel Averaging Task
    #
    cp.ingest.task.ChannelAvgTask.type                  = ChannelAvgTask
    cp.ingest.task.ChannelAvgTask.params.averaging      = 54

    #
    # Cal task
    #
    cp.ingest.task.CalTask.type    = CalTask

    # The below 4 lines configure the use of the calibration data service
    cp.ingest.task.CalTask.params.source.type           = dataservice
    cp.ingest.task.CalTask.params.source.locator_host   = localhost
    cp.ingest.task.CalTask.params.source.locator_port   = 4061
    cp.ingest.task.CalTask.params.source.service_name   = CalibrationDataService

    #
    # Measurement Set Sink
    #
    cp.ingest.task.MSSink.type                          = MSSink
    cp.ingest.task.MSSink.params.filenamebase           = ingest_test
    cp.ingest.task.MSSink.params.stman.bucketsize       = 1048576
    cp.ingest.task.MSSink.params.stman.tilencorr        = 4
    cp.ingest.task.MSSink.params.stman.tilenchan        = 1
