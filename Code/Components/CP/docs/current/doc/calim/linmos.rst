linmos (Linear Mosaic Applicator) Documentation
===============================================

This page provides instruction for using the linmos program. The purpose of
this software is to perform a linear mosaic of a set of images.

Running the program
-------------------

It can be run with the following command, where "config.in" is a file containing
the configuration parameters described in the next section. ::

   $ linmos -c config.in

The *linmos* program is not parallel/distributed.

Configuration Parameters
------------------------

The following table contains the configuration parameters to be specified in the *config.in*
file shown on above command line. Note that each parameter must be prefixed with "linmos.".
For example, the *weighttype* parameter becomes *linmos.weighttype*.

Note: during the BETA campaign there is no default option for weighttype. This option must be set.

+------------------+------------------+--------------+------------------------------------------------------------+
|**Parameter**     |**Type**          |**Default**   |**Description**                                             |
+==================+==================+==============+============================================================+
|names             |vector<string>    |none          |Names of the input images.                                  |
+------------------+------------------+--------------+------------------------------------------------------------+
|weights           |vector<string>    |null          |Optional parameter (required if using weight images). Names |
|                  |                  |              |of input images containing pixel weights. There must be one |
|                  |                  |              |weight image for each image, and the size must match.       |
+------------------+------------------+--------------+------------------------------------------------------------+
|outname           |string            |none          |Name of the output image.                                   |
+------------------+------------------+--------------+------------------------------------------------------------+
|outweight         |string            |none          |Name of output image containing pixel weights.              |
+------------------+------------------+--------------+------------------------------------------------------------+
|weighttype        |string            |none          |How to determine the pixel weights. Options:                |
|                  |                  |              |- **FromWeightImages**: from weight images. Parameter       |
|                  |                  |              |  *weights* must be present and there must be a one-to-one  |
|                  |                  |              |  correspondence with the input images.                     |
|                  |                  |              |- **FromPrimaryBeamModel**: using a Gaussian primary-beam   |
|                  |                  |              |  model. **If beam centres are not specified (see below),   |
|                  |                  |              |  the reference pixel of each input image is used.**        |
+------------------+------------------+--------------+------------------------------------------------------------+
|weightstate       |string            |Corrected     |Optional parameter. The weighting state of the input images.|
|                  |                  |              |Options:                                                    |
|                  |                  |              |- **Corrected**: Direction-dependent beams/weights have     |
|                  |                  |              |  been divided out of input images.                         |
|                  |                  |              |- **Inherent**: Input images retain the natural             |
|                  |                  |              |  primary-beam weighting of the visibilities.               |
|                  |                  |              |- **Weighted**: Full primary-beam-squared weighting.        |
+------------------+------------------+--------------+------------------------------------------------------------+
|psfref            |uint              |0             |Optional parameter. Which of the input images to extract    |
|                  |                  |              |restoring-beam information from. default is the first       |
+------------------+------------------+--------------+------------------------------------------------------------+
|If input images need to be regridded, the following ImageRegrid options are available.                           |
+------------------+------------------+--------------+------------------------------------------------------------+
|regrid.method     |string            |linear        |Optional parameter. ImageRegrid interpolation method:       |
|                  |                  |              |*nearest*, *linear* or *cubic* (possibly *lanczos* soon).   |
+------------------+------------------+--------------+------------------------------------------------------------+
|regrid.decimate   |uint              |3             |Optional parameter. ImageRegrid decimation factor. 3-10     |
|                  |                  |              |likely to be best.                                          |
+------------------+------------------+--------------+------------------------------------------------------------+
|regrid.replicate  |bool              |false         |Optional parameter. ImageRegrid *replicate* option.         |
+------------------+------------------+--------------+------------------------------------------------------------+
|regrid.force      |bool              |false         |Optional parameter. ImageRegrid *force* option.             |
+------------------+------------------+--------------+------------------------------------------------------------+

Definition of beam centres
--------------------------

If weights are generated from primary-beam models (*weighttype=FromPrimaryBeamModel*), it is possible to set the
beam centres from within the parset. Since this is most likely useful when each input image comes from a different
multi-beam feed, *feeds* offset parameters from other applications are used for this. If beam centres are not
specified, the reference pixel of each input image is used.

The *feeds* parameters can be given either in the main linmos parset or a separate offsets parset file set by the
*feeds.offsetfile* parameter. 

+------------------+------------------+--------------+------------------------------------------------------------+
|**Parameter**     |**Type**          |**Default**   |**Description**                                             |
+==================+==================+==============+============================================================+
|Parameters specified from within the main linmos parset. As above, these parameters require the *linmos.* prefix.|
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.centre      |vector<string>    |none          |Optional parameter (required when specifying beam offsets). |
|                  |                  |              |Two-element vector containing the right ascension and       |
|                  |                  |              |declination that all of the offsets are relative to.        |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.spacing     |string            |none          |Optional parameter (required when specifying beam offsets   |
|                  |                  |              |in the main linmos parset). Beam/feed spacing when giving   |
|                  |                  |              |offsets in the main linmos parset. If *feeds.offsetsfile*   |
|                  |                  |              |is given, this parameter will be ignored.                   |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.names[i]    |vector<string>    |none          |Optional parameter (required when specifying beam offsets   |
|(one per input    |                  |              |in the main linmos parset). Two-element vector containing   |
|image)            |                  |              |the beam offset relative to the *feeds.centre* parameter.   |
|                  |                  |              |Offsets correspond to hour angle and declination.           |
|                  |                  |              |*names[i]* should match the names of the input images,      |
|                  |                  |              |given in *linmos.names* (see above). If *feeds.offsetsfile* |
|                  |                  |              |is given, these parameters will be ignored.                 |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.offsetsfile |string            |none          |Optional parameter. Name of the optional beam/feed offsets  |
|                  |                  |              |parset. If present, any offsets specified in the main       |
|                  |                  |              |linmos parset will be ignored.                              |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.names       |vector<string>    |none          |Optional parameter (required either here or below when      |
|                  |                  |              |specifying a beam offset parset). The beam offset parset    |
|                  |                  |              |should have one line per input image, with parameter keys   |
|                  |                  |              |(minus the *feeds.* prefix) specified by this parameter. If |
|                  |                  |              |the offset parset also contains a *names* parameter, the    |
|                  |                  |              |main linmos entry will hold, to allow a subset of beams     |
|                  |                  |              |from a general to be chosen.                                |
+------------------+------------------+--------------+------------------------------------------------------------+
|Parameters specified from within a feeds offset parset. These parameters have only the *feeds.* prefix.          |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.names       |vector<string>    |null          |Optional parameter (required either here or above when      |
|                  |                  |              |specifying a beam offset parset). The beam offset parset    |
|                  |                  |              |should have one line per input image, with parameter keys   |
|                  |                  |              |(minus the *feeds.* prefix) specified by this parameter. If |
|                  |                  |              |the offset parset also contains a *names* parameter, the    |
|                  |                  |              |main linmos entry will hold, to allow a subset of beams     |
|                  |                  |              |from a general to be chosen.                                |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.spacing     |string            |none          |Beam/feed spacing. When using this extra offsets parset,    |
|                  |                  |              |the spacing needs to be specified in this parset.           |
+------------------+------------------+--------------+------------------------------------------------------------+
|feeds.beamnames[i]|vector<string>    |none          |Two-element vector containing the beam offset relative to   |
|(one per input    |                  |              |the *feeds.centre* parameter. Offsets correspond to hour    |
|image)            |                  |              |angle and declination. *beamnames[i]* should match the      |
|                  |                  |              |names given in feeds.names* (see above).                    |
+------------------+------------------+--------------+------------------------------------------------------------+

Example
-------

.. code-block:: bash

    # Example linmos parset to combine individual feed images from a 36-feed simulation.
    # Weights images are used to weight the pixels.

    linmos.weighttype = FromWeightImages

    linmos.names      = [image_feed00..35_offset.i.dirty.restored]
    linmos.weights    = [weights_feed00..35_offset.i.dirty]

    linmos.outname    = image_mosaic.test
    linmos.outweight  = weights_mosaic.test

.. code-block:: bash

    # Example linmos parset to combine the four inner-most feed images from a 36-feed simulation.
    # Gaussian primary-beam models are used to weight the pixels.

    linmos.weighttype = FromPrimaryBeamModel

    linmos.names      = [image_feed14..15.i.dirty.restored, image_feed20..21.i.dirty.restored]

    linmos.outname    = image_mosaic.test
    linmos.outweight  = weights_mosaic.test

    linmos.feeds.centre   = [12h30m00.00, -45.00.00.00]

    # specify a beam offset file
    linmos.feeds.offsetsfile = linmos_ingest.in
    # linmos_ingest.in has a feeds.names line, but it includes all 36 beam. So specify which ones we want here
    linmos.feeds.names       = [PAF36.feed14..15, PAF36.feed20..21]

linmos_ingest.in:
    <snip>
    feeds.PAF36.feed14          = [-0.5, -0.5]
    feeds.PAF36.feed15          = [-0.5,  0.5]
    <snip>
    feeds.PAF36.feed20          = [0.5, -0.5]
    feeds.PAF36.feed21          = [0.5,  0.5]
    <snip>

.. code-block:: bash

    # Example linmos parset to combine the four inner-most feed images from a 36-feed simulation.
    # Specify the primary-beam offsets directly.

    linmos.weighttype = FromPrimaryBeamModel

    linmos.names      = [image_feed14..15.i.dirty.restored, image_feed20..21.i.dirty.restored]

    linmos.outname    = image_mosaic.test
    linmos.outweight  = weights_mosaic.test

    linmos.feeds.centre   = [12h30m00.00, -45.00.00.00]

    linmos.feeds.spacing = 1deg
    linmos.feeds.image_feed14.i.dirty.restored = [-0.5, -0.5]
    linmos.feeds.image_feed15.i.dirty.restored = [-0.5,  0.5]
    linmos.feeds.image_feed20.i.dirty.restored = [0.5, -0.5]
    linmos.feeds.image_feed21.i.dirty.restored = [0.5,  0.5]

