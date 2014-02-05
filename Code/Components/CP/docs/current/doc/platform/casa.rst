Using CASA
==========

While the ASKAP Central Processor was not specifically designed to run CASA, there are a
number of approaches to using CASA. It can be run on the compute nodes, however there is
no X-Windows system there, so GUI tools such as casaviewer, casabrowser & casaplotms will
not work. We do have a number of (temporarily) "spare" nodes which we can temporarily
dedicate to CASA usage. These are Dell servers with 64GB RAM. However these nodes are part
of the ingest pipeline infrastructure and will be dedicated to that purpose as more
antennas come online.

.. warning:: Neither casapy nor any of the CASA GUI tools (casaviewer, casabrowser, casaplotms)
             should be executed on the login nodes (eslogin-rt000 or eslogin-rt001). Using
             these tools often results in memory exhaustion and will hang the login nodes,
             preventing all users from accessing the system.

Running interactive CASA with GUI
---------------------------------

The two nodes that have been earmarked for temporary CASA usage are *esdm014* and
*esdm015*.  These nodes cannot be accessed directly, and must be accessed via the login
nodes. The following commands will launch CASA::

    ssh -Y galaxy.ivec.org
    ssh -Y esdm014
    module load casa
    casa

Note you can choose from esdm014 or esdm015. We can add more nodes to this group later if
the need arises.  These nodes share the home directory with the Cray supercomputer itself
and also mount the /scratch filesystem.


Running CASA on Compute Nodes
-----------------------------

.. note:: CASA has not been well tested on the Cray platform, and indeed CASA is not designed 
          to be run in a HPC environment.

An interactive CASA session can be run on a compute node by submitting an *interactive*
job. To lauch an interactive CASA session on a compute node first save this script to a
file named *interactive.qsub*::

    #!/bin/bash -l
    #PBS -l walltime=24:00:00
    #PBS -l mppwidth=1
    #PBS -l mppnppn=1
    #PBS -N interactive
    #PBS -I

Then execute is like you would a normal batch job::

    qsub interactive.qsub

However instead of simply queuing the job then returning, the command will print out a message
indicating it is waiting for the job to start::

    qsub: waiting for job 1234.rtc to start

then it will block until a compute node is available. When a compute node is available (and one
may well be immediately available) you will be presented with a shell prompt::

    qsub: job 1234.rtc ready

    joe123@nid00002:~>

From here you can load the module for CASA and execute CASAPY::

    module load casa
    aprun -b casa --nologger --log2term --nogui
