msmerge (Measurement Merging Utility)
=====================================

The *msmerge* utility is used to merge measurement sets in frequency.

The intended use-cases of this tool are:

- Merge measurement sets from different frequency sub-bands into a single
  measurement set. It is used, for example, to create a single simulated
  measurement set when one needs to split a simulation into sub-bands.

Running the program
-------------------

It can be run with the following command ::

   $ msmerge.sh -o output_file list_of_input_files

The *msmerge* program is not parallel/distributed, it runs in a single process.

Configuration Parameters
------------------------

At this time *msmerge* does not accept a configuration parameter file.

Example
-------

**Example 1:**

.. code-block:: bash

   $ msmerge.sh -o fullband.ms subband_???.ms

**Example 2:**

.. code-block:: bash

   $ msmerge.sh -o output.ms channel1.ms channel2.ms channel3.ms

