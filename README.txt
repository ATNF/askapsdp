Building ASKAPsoft                                           29 July 2010
==================

These notes provide just a brief outline.
More detailed information can be found on the ASKAP project management site:
https://pm.atnf.csiro.au/askap/wiki/cmpt
and in particular:
https://pm.atnf.csiro.au/askap/wiki/cmpt/IS_Software_Build_System

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

svn co https://svn.atnf.csiro.au/askapsoft/Src/trunk ASKAPsoft
cd ASKAPsoft
/usr/bin/python2.6 bootstrap.py  # only once
. initaskap.sh                   # execute everytime a new session is started

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
Usage: rbuild [options] [<package_path>]
                                                                                                        
This is the main ASKAPsoft build command for developers. It can handle                                  
dependencies, subversion updates and changes to the build system. There are
two types of build targets: recursive [depends, install, stage, release,
signature] and non-recursive [bclean, clean, doc, format, pylint, functest,
test, deploy]. The non-recursive targets only apply to the current package,                             
while recursive targets are applies to all dependencies of the current                                  
package.  The recursive behaviour may be overridden with appropriate flag. The                          
default <package_path> is the current directory.                                                        
                                                                                                        
Options:
  -h, --help            show this help message and exit                                                                            
  -a, --autobuild       mode where dependencies/packages are computed once.                                                        
                        Additionally turns on no-update and no-recursion flags                                                     
                        i.e. -n -R                                                                                                 
  -f, --force           force building of packages ignoring NO_BUILD files                                                         
  -q, --quiet           do not show all builder output [default=True]
  -v, --verbose         show all builder output                                                                                                      
  -n, --no-update       no svn updates, rebuild of myself or Tools rebuild.                                                                          
                        Equivalent to "-S -M -T"                                                                                                     
  -N, --no-recursive-update                                                                                                                          
                        no svn updates, rebuild of myself, Tools rebuild or                                                                          
                        recursion. Equivalent to "-S -M -T -R -v" or                                                                                 
                        "python build.py TARGET"                                                                                                     
  -p EXTRAOPTS, --pass-options=EXTRAOPTS
                        pass on package specific build options, e.g. "mpi=1"                                                                         
                        or specific functional tests e.g "-t functest -p                                                                             
                        functests/mytest.py"                                                                                                         
  -t TARGET, --target=TARGET                                                                                                                         
                        select TARGET from: depends, install, stage, release,                                                                        
                        signature, bclean, clean, doc, format, pylint,
                        functest, test, deploy [default=install]
  --release-name=RELEASE_NAME
                        the name of the staging directory and the release
                        tarball
  --deploy-targets=DEPLOY_TARGETS
                        the deployement targets to execute. Select any
                        DEPLOY_TARGETS from:
                        authenticate,deploy_local,deploy_remote,verify [defaul
                        t=authenticate,deploy_local,deploy_remote,verify]

  Advanced Options:
    Caution: Use these options at your own risk.

    -D, --debug         do not run the actual package building command
    -M, --no-build-myself
                        do not rebuild myself (rbuild)
    -P, --no-parallel   do not do parallel builds of packages
    -R, --no-recursion  do not apply target recursively to dependencies
    -S, --no-svn-update
                        do not perform subversion update
    -T, --no-tools      do not rebuild Tools
    -U, --update-only   Ignore any target options and just do svn update
    -V, --no-virtualenv
                        do not include virtualenv in a release
    -X, --no-exit-on-error
                        continue building ignoring any individual package
                        build failures

------------------------------------------------------------------------------

rbuild system
=============
The rbuild system is the infrastructure underlying the rbuild command.
At the package level it can be run via the rbuild script or calling the
build.py files directly. In this case there is no recursive building.

e.g.
python build.py [-q|-x] <target>

It provides the following command-line options

-q        suppress messages to stdout and just print errors
-x        exit recursivebuild when an error is encountered

and targets  as for rbuild.


Express install (for the very brave - takes a long time)
========================================================

The following will (hopefully) automatically install Tools, 3rdParty 
and Code. If you don't want to do it all at once then skip this and
move to the next step.  
Note - The Tools subdirectory is built during the intial bootstrap
but can be rebuilt if desired to incorporate latest updates.

cd $ASKAP_ROOT; rbuild -a


Installing the Sub-hierarchies
==============================
The order is important.  Tools must be done before 3rdParty which must be
done before Code.
These command should build all packages and versions of the packages.

Installing Tools
================
cd $ASKAP_ROOT/Tools
rbuild -a

Installing 3rdParty
===================
cd $ASKAP_ROOT/3rdParty
rbuild -a


Install Code
============
cd $ASKAP_ROOT/Code
rbuild -a

Troubleshooting
===============

Make sure that you don't have a ~/.pydistutils.cfg because this
can conflict with virtualenv.
