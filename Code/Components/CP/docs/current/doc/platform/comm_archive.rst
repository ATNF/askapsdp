Commissioning Archive Platform
==============================

At the CSIRO ARRC building a large tape archive exists. The system itself can be accessed by ssh::

    $ ssh cortex.ivec.org

You have a directory on an archive file system, "/pbstore/userfs/<username>". There is a link
in your home directory called ARCHIVE that points to your personal archive directory
on the "userfs" file system. This file system has the following default quotas:

* For disk space, there is a 200GB soft limit and a 400GB hard limit.
* For number of files, there is a 20,000 file soft limit and a 30,000 hard limit.

To transfer data from epic to pbstore (when logged into epic)::

    scp <filename> <username>@cortex.ivec.org:/pbstore/userfs/<username>

To transfer data from pbstore to epic (when logged into epic)::

    scp <username>@cortex.ivec.org:/pbstore/userfs/<username>/<filename>

Note that if the files are only on tape, it is generally more efficient to log in to
cortex.ivec.org and stage the files first. A single file can be staged with the command
stage <filename> or a directory can have all files recursivly staged with the command
stage -r <filename>

To list your files on /pbstore/userfs/<username> (when logged into epic)::

    ssh <username>@/cortex/userfs/<username>.ivec.org ls -l /pbstore/userfs/<username>

