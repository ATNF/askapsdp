noiseadder
==========

Usage::

    noiseadder.sh <measurement_set> <noise_variance_in_Jy^2>

This program simply adds gaussian noise to all visibilities in the cube (i.e. to all polarisations, spectral channels and rows). The noise
level is defined by explicitly given variance. The measurement set is modified in situ. The random number generator is seeded with 
current time and MPI rank, allowing the program to be used on a cluster.


