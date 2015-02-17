mssplit (Measurement Splitting/Averaging Utility)
=================================================

The *mssplit* utility is used to extract a subset of a measurement set. This
subset may be a channel, beam id or scan id selection. It also has the ability
to average channels together while doing so. It can also be used simply to
average channels; i.e. just channel averaging, no filtering/selection.

The intended use-cases of this tool are:

- Split a large measurement with many spectral channels into many smaller
  measurement sets, with perhaps a single spectral channel per file. This
  allows the MPI-based calibration and imaging programs to read a specific
  measurement set so no selection need be done.

- Average a full spectral resolution measurement set down to fewer/wider
  channels for continuum imaging.

Additionally, *mssplit* can filter based on the following criteria:

- Scan Number
- Beam Number

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $  mssplit -c config.in

The *mssplit* program is not parallel/distributed, it runs in a single process operating
on a single input measurement set.

Configuration Parameters
------------------------

+----------------------+------------+-----------------------+---------------------------------------------+
|**Parameter**         |**Default** |**Example**            |**Description**                              |
+======================+============+=======================+=============================================+
|vis                   |*None*      |2013-12-25_230000.ms   |The input measurement set (uv-dataset). This |
|                      |            |                       |file will not be modified.                   |
|                      |            |                       |                                             |
+----------------------+------------+-----------------------+---------------------------------------------+
|outputvis             |*None*      |chan_1.ms              |The output measurement set (uv-dataset). This|
|                      |            |                       |file will be created, and the program will   |
|                      |            |                       |fail to execute in the case a file/directory |
|                      |            |                       |with the same name already exists.           |
|                      |            |                       |                                             |
+----------------------+------------+-----------------------+---------------------------------------------+
|channel               |*None*      |1-300                  |The channel range to split out into its own  |
|                      |            |                       |measurement set. Can be either a single      |    
|                      |            |                       |integer (e.g. 1) or a range (e.g. 1-300). The|
|                      |            |                       |range is inclusive of both the start and end,|
|                      |            |                       |and indexing is one-based.                   |
+----------------------+------------+-----------------------+---------------------------------------------+
|width                 |1           |54                     |Defines the number of input channels to      |
|                      |            |                       |average together to form one output channel. |
|                      |            |                       |As the averaged visiblities can have         |
|                      |            |                       |different noise levels due to flagging,      |
|                      |            |                       |an additional array column containing noise  |
|                      |            |                       |sigmas for each spectral channel will be     |
|                      |            |                       |written when width>1.                        |
+----------------------+------------+-----------------------+---------------------------------------------+
|beams                 |*None*      |[0]                    |Defines the beam numbers that will be        |
|                      |            |or                     |exported to the output files. If this        |
|                      |            |[0, 1, 2]              |parameter is not set all beams are exported. |
|                      |            |or                     |The value may be a single integer (e.g. 0 or |
|                      |            |[0..8]                 |[0]), an array of integers such as [0,1,2]   |
|                      |            |                       |or a range such as [0..8].                   |
+----------------------+------------+-----------------------+---------------------------------------------+
|scans                 |*None*      |[0]                    |Defines the scan numbers that will be        |
|                      |            |or                     |exported to the output files. If this        |
|                      |            |[0, 1, 2]              |parameter is not set all scans are exported. |
|                      |            |or                     |The value may be a single integer (e.g. 0 or |
|                      |            |[0..2]                 |[0]), an array of integers such as [0,1,2]   |
|                      |            |                       |or a range such as [0..2].                   |
+----------------------+------------+-----------------------+---------------------------------------------+
|fieldnames            |*None*      |[offset1]              |Defines the field names that will be         |
|                      |            |or                     |exported to the output files. If this        |
|                      |            |[offset1,offset2]      |parameter is not set all fields are exported.|
|                      |            |or                     |The value may be a single string (e.g. a0 or |
|                      |            |[offset1..9]           |[a0]), an array of strings such as [a0,a1,a2]|
|                      |            |                       |or a range such as [a0..2].                  |
+----------------------+------------+-----------------------+---------------------------------------------+
|timebegin             |*None*      |1996/11/20/5:20        |Defines a time based filter. Any rows with   |
|                      |            |or                     |time i*earlier than* this parameter will be  |
|                      |            |20Nov96-5h20m          |excluded during splitting (i.e. they will    |
|                      |            |or                     |not be copied to the output measurement set. |
|                      |            |1996-11-20T5:20        |This parameter is optional and if not present|
|                      |            |                       |there will be no *later than* filter applied.|
+----------------------+------------+-----------------------+---------------------------------------------+
|timeend               |*None*      |1996/11/20/5:20        |Defines a time based filter. Any rows with   |
|                      |            |or                     |time *later than* this parameter will be     |
|                      |            |20Nov96-5h20m          |excluded during splitting (i.e. they will    |
|                      |            |or                     |not be copied to the output measurement set. |
|                      |            |1996-11-20T5:20        |This parameter is optional and if not present|
|                      |            |                       |there will be no *earlier than* filter       |
|                      |            |                       |applied.                                     |
+----------------------+------------+-----------------------+---------------------------------------------+

Additional advanced/optional parameters:
````````````````````````````````````````

+----------------------+------------+-----------------------+---------------------------------------------+
|**Parameter**         |**Default** |**Example**            |**Description**                              |
+======================+============+=======================+=============================================+
|stman.bucketsize      |65536       |                       |Set the bucket size (in bytes) of the CASA   |
|                      |            |                       |Table storage manager. This usually          |
|                      |            |                       |translates into the I/O size.                |
+----------------------+------------+-----------------------+---------------------------------------------+
|stman.tilencorr       |4           |                       |Set the number of correlations per tile. This|
|                      |            |                       |affects the way the data is written to and   |
|                      |            |                       |read from disk.                              |
+----------------------+------------+-----------------------+---------------------------------------------+
|stman.tilenchan       |1           |                       |Set the number of spectral channels per tile.|
|                      |            |                       |This affects the way the data is written to  |
|                      |            |                       |and read from disk. If it is expected that a |
|                      |            |                       |given reader or writer process will read only|
|                      |            |                       |a single channel then the default value of 1 |
|                      |            |                       |is fine. If the reader or writer is expected |
|                      |            |                       |to read many, or even all channels then a    |
|                      |            |                       |larger value would be more optimal.          |
+----------------------+------------+-----------------------+---------------------------------------------+


Configuration Example
---------------------

**Example 1**

The following example demonstrates splitting out a single spectral channel,
with no averaging:

.. code-block:: bash

    # Input measurement set
    # Default: <no default>
    vis         = full.ms

    # Output measurement set
    # Default: <no default>
    outputvis   = chan1.ms

    # The channel range to split out into its own measurement set
    # Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
    # is inclusive of both the start and end, indexing is one-based. 
    # Default: <no default>
    channel     = 1

    # Defines the number of channel to average to form the one output channel
    # Default: 1
    width       = 1


**Example 2**

The following example demonstrates both splitting and averaging. Here, the lowest
numbered 54 channels are averaged together to form a single channel in the output
measurement set.

.. code-block:: bash

    # Input measurement set
    # Default: <no default>
    vis         = full-18_5kHz.ms

    # Output measurement set
    # Default: <no default>
    outputvis   = averaged_1MHz_chan_1.ms

    # The channel range to split out into its own measurement set
    # Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
    # is inclusive of both the start and end, indexing is one-based. 
    # Default: <no default>
    channel     = 1-54

    # Defines the number of channel to average to form the one output channel
    # Default: 1
    width       = 54


**Example 3**

Finally, the following example demonstrates averaging a single measurement set
with 16416 spectral channels by a factor of 54, creating a single output
measurement set. i.e. 16416 x 18.5kHz channels to 304 x 1MHz channels.

.. code-block:: bash

    # Input measurement set
    # Default: <no default>
    vis         = full-18_5kHz.ms

    # Output measurement set
    # Default: <no default>
    outputvis   = averaged_1MHz.ms

    # The channel range to split out into its own measurement set
    # Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
    # is inclusive of both the start and end, indexing is one-based. 
    # Default: <no default>
    channel     = 1-16416

    # Defines the number of channel to average to form the one output channel
    # Default: 1
    width       = 54
