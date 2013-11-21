Makecube Documentation
======================

Introduction
------------

This page provides some details about the "makecube" program located in:

| ``$ASKAP_ROOT/Code/Components/CP/pipelinetasks/current/apps/makecube.cc``

The makecube program merges multiple images into a cube. This is intended to take individual
image planes output from the spectral line imaging pipeline and merge them into a cube.
Important notes:

- All source images must have the same dimensions and coordinate systems.
- A rest frequency can be provided.
- Restoring beam (i.e. the image info) is copied from a single specified image. Unfortunately casa images don't appear to support a restoring beam that varies with frequency.
- A record of the beam sizes of the input images can be created for future reference.

Running the program
-------------------

The code should be compiled with the ASKAPsoft build system::

   $ cd $ASKAP_ROOT
   $ rbuild Code/Components/CP/pipelinetasks/current

It can then be run in the usual fashion by providing a Parameter Set
as input::

   $ $ASKAP_ROOT/Code/Components/CP/pipelinetasks/current/install/bin/makecube.sh -c parset.in


The key parameters to be provided are the output cube name, and the pattern describing the input files. Since makecube could potentially be processing thousands of input files, the input can be described using a pattern like "residual.i.[0..15].restored". This would result in images *residual.i.0.restored* to *residual.i.15.restored* (inclusive) being used to form the cube.

It is possible to specify a rest frequency to be applied to the output cube (since *cimager* will not provide this). A common value for this, that of the fine-structure line of neutral hydrogen (HI), or 1420405751.786 Hz, can be requested by setting the rest frequency to "HI" in the parameter set. If no rest frequency is provided, or a negative value is given, no rest frequency will be written to the cube.

The image info (which essentially means the beam information) for the output cube is copied from a designated input image. This is, by default, the middle image of the range (specifically, (number of images) / 2, with integer division), but this can be changed by setting *beamReference* to 'first', 'last', or a number in the range. The individual beam sizes for each of the input images (assuming they are defined) can be written to an ascii text file for future reference. The file has columns: index | image name | major axis [arcsec] | minor axis [arcsec] | position angle [deg]
Here is an example of the start of a beam log::

  #Channel Image_name BMAJ[arcsec] BMIN[arcsec] BPA[deg]
  0 image.i.cube.clean_ch0.restored 65.7623 36.4139 -54.7139
  1 image.i.cube.clean_ch1.restored 65.7981 36.4222 -54.7605
  2 image.i.cube.clean_ch2.restored 65.8365 36.4324 -54.7985
  3 image.i.cube.clean_ch3.restored 65.8733 36.4413 -54.845
  4 image.i.cube.clean_ch4.restored 65.9082 36.4512 -54.8919

The following table describes the possible parameters.

+--------------------------+-------------+------------+----------------------------------------------------------------+
|*Parameter*               |*Type*       |*Default*   |*Explanation*                                                   |
+==========================+=============+============+================================================================+
|Makecube.outputCube       |string       |""          |Name of the spectral cube to be created.                        |
+--------------------------+-------------+------------+----------------------------------------------------------------+
|Makecube.inputNamePattern |string       |""          |The pattern describing the input names. See text for details.   |
+--------------------------+-------------+------------+----------------------------------------------------------------+
|Makecube.restFrequency    |string/float |-1.         |The rest frequency to be written to the cube's coordinate       |
|                          |             |            |system. Negative values mean nothing is written. Provide either |
|                          |             |            |a numerical value or the string "HI" (which is equivalent to    |
|                          |             |            |1420405751.786).                                                |
+--------------------------+-------------+------------+----------------------------------------------------------------+
|Makecube.beamReference    |string/int   |mid         |Which of the input images to get the beam information           |
|                          |             |            |from. Options include: 'mid' (middle image of list), 'first',   |
|                          |             |            |'last', or a number indicating the image (list is zero-based).  |
+--------------------------+-------------+------------+----------------------------------------------------------------+
|Makecube.beamLog          |string       |""          |Name of the ascii text file to which the beam information for   |
|                          |             |            |every input file is written.                                    |
+--------------------------+-------------+------------+----------------------------------------------------------------+

The following demonstrates a parset for a continuum cube (no rest frequency)::

  Makecube.inputNamePattern = image.i.cube.clean_ch[0..151].restored
  Makecube.outputCube = image.i.cube.clean.restored
  Makecube.restFrequency = -1.
  Makecube.beamReference = mid
  Makecube.beamLog = beamFile.image.i.cube.clean.restored.dat

and the following demonstrates a parset for a small spectral-line cube focussed on HI emisison::

 Makecube.inputNamePattern = image.i.SLtute.cube_ch[0..31].restored
 Makecube.outputCube = image.i.SLtute.cube.restored
 Makecube.restFrequency = HI
 Makecube.beamReference = mid
 Makecube.beamFile = beamFile.image.i.SLtute.cube.restored.dat
