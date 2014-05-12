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
(all parameters must have *Cdeconvolver* prefix, i.e.
*Cdeconvolver.solver.algorithm*). 

+------------------------------+------------+--------------------+-----------------------------------------+
|*Parameter*                   |*Type*      |*Default*           |*Description*                            |
+==============================+============+====================+=========================================+
|dirty                         |string      |none                |input dirty image                        |
|                              |            |                    |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
|psf                           |string      |none                |input point spread function image        |
|                              |            |                    |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
|weight                        |string      |none                |input weight image                       |
|                              |            |                    |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
|model                         |string      |none                |output model                             |
|                              |            |                    |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
|residual                      |string      |none                |output residual image                    |
|                              |            |                    |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
|solver                        |string      |"Fista"             |Name of the solver. Currently supported  |
|                              |            |                    |options are "Fista","Entropy", "Clean"   |
|                              |            |                    |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
|solver.solver_name.algorithm  |string      |"Hogbom,            |algorithm option for a solver            |
|                              |            |Basisfunction" for  |                                         |
|                              |            |"Clean", "EntropyI, |                                         |
|                              |            |Emptiness" for      |                                         |
|                              |            |Entropy             |                                         |
+------------------------------+------------+--------------------+-----------------------------------------+
