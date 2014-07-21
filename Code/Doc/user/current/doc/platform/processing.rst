Central Processor Platform
==========================

Overview
--------

The ASKAP Central Processor platform consists of a Cray XC30 supercomputer and a 1.4PB Cray
Sonexion storage system mounted as a Lustre filesystem.

* 2 x Login Nodes:

  - 2 x 2.4 GHz Intel Xeon E5-2665 (Sandy Bridge) CPUs
  - 64 GB RAM
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

* High-Performance Storage

  - 1.4PB Lustre Filesystem
  - Approximately 25GB/s I/O performance


Login
------
To login to the ASKAP Central Processor::

   ssh abc123@galaxy.ivec.org

Where abc123 is the login name iVec gave you. In that case that your CSIRO login differers
from your iVec login, and to avoid having to specify the username each time, you can add
the following to your ~/.ssh/config file::

   Host *.ivec.org
     User abc123

Setting up your account
-------------------------
Add the following to your ~/.bashrc

.. code-block:: bash

    # Use the ASKAP environment modules collection
    module use /ivec/askap/modulefiles

    # Load the ASKAPsoft module
    module load askapsoft

    # Load the measures data
    module load askapdata

    # Load the BBCP module for fast external data transfer
    module load bbcp

    # Allow MPICH to fallback to 4k pages if large pages cannot be allocated
    export MPICH_GNI_MALLOC_FALLBACK=enabled

Local Filesystems
-----------------

You have two filesystems available to you:

* Your home directory
* The *scratch* filesystem, where you have a directory /scratch/askap/$USER

The scratch filesystem should be used for executing your processing jobs. Specifically
bulk data (measurement sets, images, etc) to be read from the job, and all output written,
should be from/to the scratch filesystem.

Note that your home directory, while it can be read from the compute nodes, cannot be
written to from the compute nodes. It is mounted read-only on the compute nodes to prevent
users from being able to "clobber" the home directory server with thousands of concurrent I/Os
from compute nodes.

Submitting a job:
-----------------

This section describes the job execution environment on the ASKAP Central Processor. The
system uses SLURM for Job scheduling, however the below examples use a Cray specific
customisation to declare the resources required. An example qsub file is::

    #!/usr/bin/env bash
    #SBATCH --time=01:00:00
    #SBATCH --ntasks=80
    #SBATCH --ntasks-per-node=20
    #SBATCH --mail-user=my.email.address@csiro.au
    #SBATCH --job-name=myjobname
    #SBATCH --mail-type=FAIL
    #SBATCH --no-requeue
    #SBATCH --export=NONE

    aprun -B ./myprogram

Note the use of *aprun* instead of *mpirun*. The -B option to *aprun* tells ALPS (the
Application Level Placement Scheduler) to reuse the width, depth, nppn and memory requests
specified with the corresponding batch reservation.

Specifically, the following part of the above file requests 80 processing
elements (PE) to be created. A PE is just a process. The parameter *ntasks-per-node*
says to execute 20 PEs per node, so this job will require 4 nodes (80/20=4)::

    #SBATCH --ntasks=80
    #SBATCH --ntasks-per-node=20

Then to submit the job::

    sbatch myjob.qsub


Other example resource specifications
-------------------------------------

The following example launches a job with a number of PEs that is not a multiple of
*ntasks-per-node*, in this case 22 PEs::

    #!/usr/bin/env bash
    #SBATCH --time=01:00:00
    #SBATCH --ntasks=22
    #SBATCH --ntasks-per-node=20
    #SBATCH --mail-user=my.email.address@csiro.au
    #SBATCH --job-name=myjobname
    #SBATCH --mail-type=FAIL
    #SBATCH --no-requeue
    #SBATCH --export=NONE

    aprun -n 22 -N 20 ./myprogram

Note that instead of passing "-B", which says use the numbers from *ntasks* & *ntasks-per-node*,
you must pass "-n" and "-N" specifically. Using the "-B" option only works if *ntasks* is
divisible by *ntasks-per-node*.

**OpenMP Programs:**

The following example launches a job with 20 OpenMP threads per process (although there is only
one process). The *cpus-per-task* option declares the number of threads to be allocated
per process.  The below example starts a single PE with 20 threads::

    #!/usr/bin/env bash
    #SBATCH --time=00:30:00
    #SBATCH --ntasks=1
    #SBATCH --cpus-per-task=20
    #SBATCH --job-name=myjobname
    #SBATCH --export=NONE

    # Instructs OpenMP to use 20 threads
    export OMP_NUM_THREADS=20

    aprun -B ./my_openmp_program


Monitoring job status
---------------------

To see your incomplete jobs::

    squeue -u $USER

Sometimes it is useful to see the entire queue, particularly when your job is queued and you wish
to see how busy the system is. The following commands show running jobs::

    squeue 
    apstat

And to display accounting information, that includes completed jobs, the following command
can be used::

    sacct

Additional Information
----------------------

* `Cray XC30 System Documentation <http://docs.cray.com/cgi-bin/craydoc.cgi?mode=SiteMap;f=xc_sitemap>`_
* `SLURM Homepage <http://computing.llnl.gov/linux/slurm>`_
* `Migrating from PBS to SLURM <https://portal.ivec.org/docs/Supercomputers/Migrating_from_PBS_Pro_to_SLURM>`_
