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
'recursivebuild' provides the following command-line options

install   install the packages
clean     clean up the build directories (doesn't remove the installed eggs)
doc       build documentation
test      run unit tests
pylint    run pylint on the module
-q        suppress messages to stdout and just print errors
-x        exit recursivebuild when an error is encountered

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

Release Process
===============
One of the primary motivations for all of the above procedure is to facilitate
releasing tagged versions of projects. To support this a releaseprocess toolbox
is provided in Tools. A project can create a release by providing a release.py
script that internally calls releaseprocess.release.

This will create a tar file that can be deployed to a destination machine, untarred
and then installed. Currently the convention is to run the install out of a sandbox
rather than embedding many files in standard locations on the destination machine.
This makes it easier to remove an installed system.

A number of conventions should be adhered to when creating the release.py script in
order to ensure a predictable experience when installing a system. With that in
mind, it is intended that an install should consist of the following steps:
1. Untar the tar file
2. cd into the tar file
3. execute ./complete.sh
4. enter root password if prompted, or confirm install options

The following assumptions are made:
1. services will be automatically started by the install and that previously running 
   versions will be removed. Parameters for service scripts should be written in 
   /etc/default/servicename to correspond with the service script in 
   /etc/init.d/servicename.
2. services will log to /var/log/servicename.log
3. deleting the untarred directory will gracefully disable the service because it should
   contain a test for the necessary files. Ideally the default, init.d and log files 
   could be removed too.

More details are provided with the different versions of releaseprocess at 
Tools/releaseprocess. They can be read by executing python setup.py doc or examining the 
source code directly for the relevant version.