Building ASKAPsoft                                           17 June 2009
==================

These notes provide just a brief outline.
More detailed information can be found on the ASKAPsoft Trac site
https://svn.atnf.csiro.au/trac/askapsoft/wiki/
and in particular
https://svn.atnf.csiro.au/trac/askapsoft/wiki/AS05_BuildInfraStructure/AskapSoftwareDevelopmentEnvironment

Introduction
============

A core assumption of the build process is that many different baseline
versions of the same libraries will exist. To accommodate this it has been 
decided that python packages will be deployed using eggs. In order for this
to work it is necessary to install the 'setuptools' package.

Setting up the ASKAP environment
================================

This procedure sets up the environment for development. The entry
point for the repository is $ASKAP_ROOT.

 * create a initaskap.sh script which sets up the unix environment
   variables. This should be executed every time a user logs in.

 * install the python tools necessary to build the system.  This is
   using 'virtualenv' to provide a python environment independent of 
   the system, 'setuptools' to install python packages and the ASKAP 
   package 'rbuild', which handles the build process.

Quick Start
===========

svn co https://svn.atnf.csiro.au/askapsoft/trunk ASKAPsoft
cd ASKAPsoft
python2.6 bootstrap.py  # only once
. initaskap.sh           # execute everytime a new session is started

The above steps make installs go to the ASKAPsoft hierarchy
$ASKAP_ROOT/{bin, lib/python<version>} and thus do not require
root permission.

rbuild
======

The 'rbuild' command is the main build command for developers.
It has the ability of updating from the subversion repository and
also recursively resolve, update and build dependencies.

To get help for 'rbuild' simply type 

------------------------------------------------------------------------------
%% rbuild -h
Usage: rbuild [options] <package_path>

The ASKAP build command for users/developers. It handles dependencies and
performs updates from subversion. Note - the subversion update only applies to
the following TARGETs [doc, extract, install, release].

Options:
  -h, --help            show this help message and exit
  -n, --no-update       do not perform subversion update
  -q, --quiet           do not show all builder output [default=True]
  -v, --verbose         show all builder output
  -p EXTRAOPTS, --pass-options=EXTRAOPTS
                        pass on package specific build options, e.g. 'mpich=1'
  -t TARGET, --target=TARGET
                        select TARGET from: clean, format, functest, pylint,
                        test, doc, extract, install, release [default=install]

  Internal Options:
    Caution: The following options are for scripts internal use only.  Use
    these options at your own risk.

    --update-myself     [default=True]

------------------------------------------------------------------------------

rbuild system
=============
The rbuild system is the infrastructure underlying the rbuild command.
At the package level it can be run via the command 

python build.py [-q|-x] <target>

It provides the following command-line options

-q        suppress messages to stdout and just print errors
-x        exit recursivebuild when an error is encountered

and targets

install   install the packages
clean     clean up the build directories (doesn't remove the installed eggs)
doc       build documentation
test      run unit tests
pylint    run pylint on the module


Express install (for the very brave - takes a long time)
========================================================

The following will (hopefully) automatically install Tools, 3rdParty 
and Code. If you don't want to do it all at once then skip this and
move to the next step.  
Note - The Tools subdirectory is built during the intial bootstrap
but can be rebuilt if desired to incorporate latest updates.

cd $ASKAP_ROOT; python autobuild.py -q install


Installing the Sub-hierarchies
==============================
The order is important.  Tools must be done before 3rdParty which must be
done before Code.
These command should build all packages and versions of the packages.

Installing Tools
================
cd $ASKAP_ROOT/Tools
python autobuild.py -q install

Installing 3rdParty
===================
cd $ASKAP_ROOT/3rdParty
python autobuild.py -q install

Install Code
============
cd $ASKAP_ROOT/Code
python autobuild.py -q install

Troubleshooting
===============

Make sure that you don't have a ~/.pydistutils.cfg because this
can conflict with virtualenv.
