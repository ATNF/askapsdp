Measurement Set Sink
====================

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
