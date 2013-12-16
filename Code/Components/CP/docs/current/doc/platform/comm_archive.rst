Commissioning Archive Platform
==============================

Within the Pawsey Centre iVEC's legacy tape archive system exists. This will be used
for the commissioning archive until such times as the newly procured Spectralogic tape archive
is available. The system itself can be accessed by ssh::

    ssh cortex.ivec.org

However you will usually be logged into the ASKAP Central Processor and want to copy data to/from
PBStore, so the examples below are from the perspective of a user logged into the ASKAP Central
Processor, or another system potentially external to the Pawsey Centre.

You have a directory on an archive file system, "/pbstore/userfs/<username>". There is a link
in your home directory called ARCHIVE that points to your personal archive directory
on the "userfs" file system. This file system has the following default quotas:

* For disk space, there is a 200GB soft limit and a 400GB hard limit.
* For number of files, there is a 20,000 file soft limit and a 30,000 hard limit.

To transfer data from the CP to PBStore::

    scp <filename> cortex.ivec.org:/pbstore/userfs/<username>

To transfer data from PBStore to the CP::

    scp cortex.ivec.org:/pbstore/userfs/<username>/<filename>

*IMPORTANT:* If you are retrieving more than one file, it is much more efficient to stage
the files before copying. Essentially by doing this you are telling PBStore, in advance,
all the files you are going to retrieve so it can fetch them in the most optimal manner.

A single file can be staged with the command stage <filename> or a directory can have
all files recursivly staged with the command stage -r <filename>. For example, to stage
a directory (actually the files within the directory) then copy retrieve it::

    ssh cortexivec.org /opt/SUNWsamfs/bin/stage -r /pbstore/userfs/<username>/mydir
    scp -r cortex.ivec.org:/pbstore/userfs/<username>/mydir

To list your files on /pbstore/userfs/<username> (when logged into epic)::

    ssh cortex.ivec.org ls -l /pbstore/userfs/<username>

