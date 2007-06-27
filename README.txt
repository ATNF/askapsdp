Building Python Projects
========================

A core assumption of the build process is that many different baseline
versions of the same libraries will exist. To accommodate this it has been 
decided that python packages will be deployed using eggs. In order for this
to work it is necessary to install the 'setuptools' package.

Setting up the CONRAD environment
=================================

This procedure sets up the environment for development. The entry
point for the repository is $CONRAD_PROJECT_ROOT.

 * create a initconrad.sh script which sets up the unix environment
   variables. This should be executed every time a user logs in.

 * install the python tools necessary to build the system.  This is
   using a modified version of 'working-env' to provide a python
   environment independent of the system, 'setuptools' to install
   python packages and the CONRAD package 'recursivebuild', which
   handles the build process.

cd svnCONRAD
python initenv.py      # only once
. initconrad.sh		   # execute everytime a new session is started
python bootstrap.py    # only once

The above steps make installs go to the local directories
$CONRAD_PROJECT_ROOT/{bin, lib/python<version>} which do not require
root permission.

recursivebuild
==============
'recursivebuild' provides four options

-q        suppress messages to stdout and just print errors
install   install the packages
doc       build documentation
test      run unit tests

Express install (for the very brave - takes a long time)
========================================================

The following will (hopefully) automatically install Tools, 3rdParty 
and Code. If you don't want to do it all at once then skip this and
move to the next step.  

cd $CONRAD_PROJECT_ROOT; python setup.py -q install

Installing Tools
================
cd $CONRAD_PROJECT_ROOT/Tools
python setup.py -q install

Installing 3rdParty
===================
cd $CONRAD_PROJECT_ROOT/3rdParty
python setup.py -q install

Install Code
============
cd $CONRAD_PROJECT_ROOT/Code
python setup.py -q install

Additional repositories (eg svnComputing/AutoBuild)
===================================================
Build first Tools, then 3rdParty and finally the Code directory. In
each case the command is:
python setup.py -q install

