Environment Modules
===================

The `Environment Modules`_ software is used to manage the user's environment, dynamically
modifying the user's environment to allow the execution of various software packages.
Both the Cray software and the ASKAP specific software uses this approach. Typically,
modules will alter the user's PATH and/or LD_LIBRARY_PATH, and other environment variables
that may be necessary to execute some software package.

.. _Environment Modules: http://modules.sourceforge.net/

You can browse all software packages that can be *loaded* using this mechanism with the
following command::

    module avail

In order for the ASKAP specific modules to appear in this list you must have already
executed the following command. However this command should have been added to your
*.bashrc* file per the instructions in the section :doc:`processing`::

    module use /group/askap/modulefiles

The ASKAP modules appear in their own section in the output from the *module avail*
command::

    ----------------------------- /ivec/askap/modulefiles ------------------------------
    askapdata/r8523(default)     askapsoft/r20948(default)    casa/41.0.24668-001(default)
    askapsoft/r20818             bbcp/13.05.03.00.0(default)

These modules are:

* **askapsoft** - The ASKAPsoft Central Processor applications binaries and libraries
* **askapdata** - Measures data for ASKAPsoft
* **casa** - NRAO's CASA software package
* **bbcp** - BBCP Fast file copy

You can use the *module whatis* command to obtain a description of the module::

    $ module whatis askapdata
    askapdata            : Measures data for ASKAPsoft

You can use the *module list* command to view already loaded modules, and you will note
many Cray modules already loaded. A given module like so::

    module load askapsoft

And can unload it just as simply::

    module unload askapsoft

Notice the *askapsoft* module has two versions, *askapsoft/r20948* and *askapsoft/r20818*.
You will also see one is tagged *"(default)"*". If you execute the *"module load askapsoft"*
command you will load the default version. You can load a specific version by specifying
the version in the *module load* command::

    module load askapsoft/r20818

Often it is useful to understand exactly how a module will modify your environment. The
*module display* command can be used to determine this. For instance::

    $ module display askapsoft
    -------------------------------------------------------------------
    /ivec/askap/modulefiles/askapsoft/r20948:

    module-whatis    ASKAPsoft software package 
    prepend-path     PATH /ivec/askap/askapsoft/r20948/bin 
    prepend-path     LD_LIBRARY_PATH /ivec/askap/askapsoft/r20948/lib64 
    -------------------------------------------------------------------

The above shows that the askapsoft module will prepend the ASKAPsoft *bin* directory to
you *PATH* environment variable, and the *lib64* directory to your *LD_LIBRARY_PATH*.
Likewise the askapdata module can be inspected::

    $ module display askapdata
    -------------------------------------------------------------------
    /ivec/askap/modulefiles/askapdata/r8523:

    module-whatis   Measures data for ASKAPsoft 
    setenv          AIPSPATH /ivec/askap/askapdata/r8523 
    -------------------------------------------------------------------

This shows that the AIPSPATH will be set to point to the directory containing measures
data.
