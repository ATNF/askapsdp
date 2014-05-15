External Data Transfer
======================

You will often want to transfer data between the ASKAP Central Processor and computers
at your home institution. For example, upon completion of imaging you may want to visualise
those images on your desktop computer.

You may use the secure copy program (scp) to copy data to/from either your home directory or
the scratch filesysyem. A special node has been setup, with hostname *galaxydata.ivec.org*,
for this purpose. It is possible to copy data through the login nodes, however this should
be avoided if possible so as to reduce the load on the login nodes.

Here is an example of copying a file from both the home directory and the scratch filesystem.
Note these commands are executed on your local host (e.g. your desktop or laptop)::

    scp -r galaxydata.ivec.org:/scratch/askap/user123/image.fits .
    scp -r galaxydata.ivec.org:~/1934-638.ms .

or to copy from your laptop/desktop to the Central Processor::

    scp -r image.fits galaxydata.ivec.org:/scratch/askap/user123
    scp -r 1934-638.ms galaxydata.ivec.org:~

Note the "-r" performs a recursive copy so can be used to copy files or directories.

Using BBCP
----------

Using *scp* can be quite slow and a program called *bbcp* is suggested for large files.

* Binary packages: http://www.slac.stanford.edu/~abh/bbcp/bin/
* Obtaining the source with git: "git clone http://www.slac.stanford.edu/~abh/bbcp/bbcp.git"
* Command line parameters: http://www.slac.stanford.edu/~abh/bbcp/

The ASKAP software team can supply Debian packages. Usage is similar to scp, but with
a few extra parameters. To copy a file from the /scratch filesystem::

    bbcp -z -P 10 -s 32 -w 2M -r galaxydata1.pawsey.ivec.org:/scratch/askap/user123/image.fits .

and to copy a file to the /scratch filesystem::

    bbcp -P 10 -s 32 -w 2M -r image.fits galaxydata1.pawsey.ivec.org:/scratch/askap/user123

.. note:: The hostname necessary to use bbcp is *galaxydata1.pawsey.ivec.org*. This is one of the
          two hosts to which the *galaxydata* DNS alias points to (the other is *galaxydata2*).
          This is necessary as bbcp doesn't reliably establish connections via the galaxydata
          alias due to the fact connections are round-robined between the two nodes.

The three additional options result in production of progress messages every 10 seconds,
sets the number of parallel network streams to be used for the transfer to 32, and sets the
preferred size of the TCP window to 2MB. On some networks increasing the number of streams
from 32 (up to a maximum of 64) may result in even higher throughput.

