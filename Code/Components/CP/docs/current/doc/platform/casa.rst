Using CASA
==========

An interactive CASA session can be run on a compute node by submitting an *interactive*
job.

.. warning:: Neither casapy nor any of the CASA GUI tools (casaviewer, casabrowser, casaplotms)
             should be executed on the login nodes. Using these tools often results in memory
             exhaustion and will hang the login nodes, preventing all users from accessing the
             system.

.. note:: CASA has not been well tested on the Cray platform, and indeed CASA is not designed 
          to be run in a HPC environment. Use it at your own risk!

To lauch an interactive CASA session on a compute node first save this script to a file named
*interactive.qsub*::

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
