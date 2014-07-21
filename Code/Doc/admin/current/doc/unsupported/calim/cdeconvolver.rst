cdeconvolver
============

This software is originally intended to debug minor cycle solvers by running
them in a stand alone fashion. Essentially, this application just configures the
solver via factory taking inputs from provided parset file and executes the
solver on casa images stored on disk (rather than from normal equations). The
resulting model is also written into a casa image. There is no intention for
this application to be a part of production system, however the same factory
call and the solver code will be embedded in the final system.

Parset parameters understood by cdeconvolver are given in the following table
(all parameters must have *Cdeconvolver* prefix, e.g.
*Cdeconvolver.solver.Clean.algorithm*). 

+------------------------------+-------------+--------------------+-----------------------------------------+
|*Parameter*                   |*Type*       |*Default*           |*Description*                            |
+==============================+=============+====================+=========================================+
|dirty                         |string       |none                |input dirty image                        |
|                              |             |                    |                                         |
+------------------------------+-------------+--------------------+-----------------------------------------+
|psf                           |string       |none                |input point spread function image.       |
|                              |             |                    |Needs to be normalised.                  |
+------------------------------+-------------+--------------------+-----------------------------------------+
|weight                        |string       |none                |input weight image                       |
|                              |             |                    |                                         |
+------------------------------+-------------+--------------------+-----------------------------------------+
|model                         |string       |none                |output model                             |
|                              |             |                    |                                         |
+------------------------------+-------------+--------------------+-----------------------------------------+
|residual                      |string       |none                |output residual image                    |
|                              |             |                    |                                         |
+------------------------------+-------------+--------------------+-----------------------------------------+
|solver                        |string       |none                |Name of the solver. Currently supported  |
|                              |             |                    |options are "Fista", "Entropy", "Clean". |
|                              |             |                    |Further parameters are given by          |
|                              |             |                    |solver.solver_name.something.            |
|                              |             |                    |See Solver Documentation for details.    |
+------------------------------+-------------+--------------------+-----------------------------------------+
|solver.solver_name.algorithm  |string       |"Basisfunction" for |Algorithm option for a solver.           |
|                              |             |Clean, "Emptiness"  |"Hogbom" or "Basisfunction" for Clean,   |
|                              |             |for Entropy         |"EntropyI" or "Emptiness" for Entropy.   |
+------------------------------+-------------+--------------------+-----------------------------------------+
|solver.solver_name.beam       |vector<float>|none                |Parameters for Gaussian restore beam     |
|                              |             |                    |[major (pixels), minor (pix), PA (deg)]  |
+------------------------------+-------------+--------------------+-----------------------------------------+

Examples
--------

**Example 1:**

Example cdeconvolver parset to deconvolve image residual.dirty using PSF image psf.dirty.norm.

.. code-block:: bash

    Cdeconvolver.dirty                      = residual.dirty
    Cdeconvolver.psf                        = psf.dirty.norm
    Cdeconvolver.weight                     = weights.dirty

    Cdeconvolver.model                      = image.cdecor
    Cdeconvolver.residual                   = residual.cdecor

    Cdeconvolver.solver                     = Clean
    Cdeconvolver.solver.Clean.algorithm     = Hogbom
    Cdeconvolver.solver.Clean.gain          = 0.1
    Cdeconvolver.solver.Clean.niter         = 100
    Cdeconvolver.solver.Clean.beam          = [5, 5, 0.0]


