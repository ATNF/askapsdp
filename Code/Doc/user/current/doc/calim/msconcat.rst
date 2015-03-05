msconcat (Measurement Concatenation Utility)
============================================

The *msconcat* utility is used to concatenate measurement sets.

The intended use-cases of this tool are:

- Concatenate measurement sets from different observations into a single
  measurement set. It is used, for example, to create a single
  measurement set from multiple scheduling blocks.

See documentation on casacore task *MSConcat* for more information.

Running the program
-------------------

It can be run with the following command ::

   $ msconcat.sh -o output_file list_of_input_files

The *msconcat* program is not parallel/distributed, it runs in a single process.

Configuration Parameters
------------------------

At this time *msconcat* does not accept a configuration parameter file.

Example
-------

**Example 1:**

.. code-block:: bash

   $ msconcat.sh -o combo.ms block_???.ms

**Example 2:**

.. code-block:: bash

   $ msconcat.sh -o cat.ms sb1.ms sb2.ms sb3.ms

