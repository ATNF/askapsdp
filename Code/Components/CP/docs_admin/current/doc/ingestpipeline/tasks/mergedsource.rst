Merged Source
=============

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
