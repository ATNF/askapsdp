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
- Restoring beam (i.e. the image info) is copied from the image at the mid-point of the cube. Unfortunately casa images don't appear to support a restoring beam that varies with frequency.

Running the program
-------------------

The code should be compiled with the ASKAPsoft build system:

::

   $ cd $ASKAP_ROOT
   $ rbuild Code/Components/CP/pipelinetasks/current

It can then be run as follows. This then uses the images *residual.i.0.restored* to *residual.i.15.restored* (inclusive)
in making up the cube named *residual.i.cube.restored*. **Note the pattern must be quoted.**

::

   $ makecube "residual.i.[0..15].restored" residual.i.cube.restored
