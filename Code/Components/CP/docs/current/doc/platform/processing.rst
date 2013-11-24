Central Processor Platform
==========================

Overview
--------

The ASKAP Central Processor platform consists of a Cray XC30 supercomputer and a 1.4PB Cray
Sonexion storage system mounted as a Lustre filesystem.

* 2 x Login Nodes:

  - 2 x 2.6 GHz Intel Xeon E5-2670 (Sandy Bridge) CPUs
  - 32 GB RAM
  - 4x QDR Infiniband connectivity to Lustre filesystem

* 472 x Cray XC30 Compute Nodes:

  - 2 x 3.0 GHz Intel Xeon E5-2690 v2 (Ivy Bridge) CPUs
  - 10 Cores per CPU (20 per node)
  - 64GB DDR3-1866Mhz RAM
  - Cray Aries (Dragonfly Topology) Interconnect

* 16 x Ingest Nodes

  - 2 x 2.0 GHz Intel Xeon E5-2650 (Sandy Bridge) CPUs
  - 64GB RAM
  - 10 GbE connectivity to MRO
  - 4x QDR Infiniband connectivity to compute nodes and Lustre filesystem

* 2 x Data Mover Nodes (for external connectivity)

  - 2 x 2.0 GHz Intel Xeon E5-2650 (Sandy Bridge) CPUs
  - 64GB RAM
  - 10 GbE connectivity to iVEC border router
  - 4x QDR Infiniband connectivity to Lustre filesystem

* High-performance Storage

  - 1.4PB Lustre Filesystem
  - Approximately 25GB/s I/O performance


Login
------
To login to the RTC::

   ssh abc123@rtc.ivec.org

Where abc123 is the login name iVec gave you. In that case that your CSIRO login differers
from your iVec login, and to avoid having to specify the username each time, you can add
the following to your ~/.ssh/config file::

   Host *.ivec.org
     ForwardX11 yes
     User abc123

Setting up your account
-------------------------
Add the following to your ~/.bashrc

.. code-block:: bash

    # The default umask is 0022, should be 0027
    umask 0027

    # Use GNU Compilers rather than Cray Compilers
    module swap PrgEnv-cray PrgEnv-gnu

    # Load additional modules
    module load askapsoft
    module load java
    export JAVA_HOME=${JAVA_PATH}

    # Allow MPICH to fallback to 4k pages if large pages cannot be allocated
    export MPICH_GNI_MALLOC_FALLBACK=enabled

Local Filesystems
-----------------

You have two filesystems available to you:

* Your home directory
* The *scratch* filesystem, where you have a directory /scratch/$USER

The scratch filesystem should be used for executing your processing jobs. Specifically
bulk data (measurement sets, images, etc) to be read from the job, and all output written,
should be from/to the scratch filesystem.

Note that your home directory, while it can be read from the compute nodes, cannot be
written to from the compute nodes. It is mounted read-only on the compute nodes to prevent
users from being able to "clobber" the home directory server with thousands of concurrent I/Os
from compute nodes.

Submitting a job:
-----------------

Magnus uses PBSPro for Job scheduling, however the below examples use a Cray specific
customisation to declare the resources required. An example qsub file is::

    #!/bin/bash -l
    #PBS -l walltime=01:00:00
    #PBS -l mppwidth=80
    #PBS -l mppnppn=20
    #PBS -M my.email.address@csiro.au
    #PBS -N myjobname
    #PBS -m a
    #PBS -j oe
    #PBS -r n

    cd $PBS_O_WORKDIR

    aprun -B ./myprogram

Note the use of *aprun* instead of *mpirun*. The -B option to *aprun* tells ALPS (the
Application Level Placement Scheduler) to reuse the width, depth, nppn and memory requests
specified with the corresponding batch reservation.

Specifically, the following part of the above file requests 80 processing elements (PE) to
be created. A PE is just a process. The parameter *mppnppn* says to execute 20 PEs per node,
so this job will require 4 nodes (80/20=4)::

    #PBS -l mppwidth=80
    #PBS -l mppnppn=20

Then to submit the job::

    qsub myjob.qsub


Other example resource specifications
-------------------------------------

The following example launches a job with a number of PEs that is not a multiple of *mppnppn*,
in this case 22 PEs::

    #!/bin/bash -l
    #PBS -l walltime=01:00:00
    #PBS -l mppwidth=22
    #PBS -l mppnppn=20
    #PBS -M my.email.address@csiro.au
    #PBS -N myjobname
    #PBS -m a
    #PBS -j oe
    #PBS -r n

    cd $PBS_O_WORKDIR

    aprun -n 22 -N 20 ./myprogram

Note that instead of passing "-B", which says use the numbers from *mppwidth* & *mppnppn*, you must pass
"-n" and "-N" specifically. Using the "-B" option only works if *mppwidth* is divisible by *mppnppn*.

**OpenMP Programs:**

The following example launches a job with 20 OpenMP threads per process (although there is only
one process). The *mppdepth* option declares the number of threads to be launched and also sets
the OMP_NUM_THREADS environment variable to be equal to *mppdepth*. The below example starts a
single PE with 20 threads::

    #!/bin/bash -l
    #PBS -l walltime=00:30:00
    #PBS -l mppwidth=1
    #PBS -l mppdepth=20
    #PBS -N jobname
    #PBS -j oe

    cd $PBS_O_WORKDIR

    aprun -B ./my_openmp_program


Monitoring job status
---------------------

To see your incomplete jobs::

    qstat -u $USER

To see detail pertaining to one particular job, run the above command, then using the job ID ask
for full information::

    qstat -f <jobid>

Sometimes it is useful to see the entire queue, particularly when your job is queued and you wish
to see how busy the system is. The following commands show running jobs::

    qstat 
    qstat -a
    apstat

Additional Information
----------------------

* `Cray XC30 System Documentation <http://docs.cray.com/cgi-bin/craydoc.cgi?mode=SiteMap;f=xc_sitemap>`_
* `PBS Professional 12.1 Users Guide (PDF) <http://resources.altair.com/pbs/documentation/support/PBSProUserGuide12.1.pdf>`_
