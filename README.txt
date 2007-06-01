Building Python Projects
========================

A core assumption of the build process is that many different baseline
versions of the same libraries will exist. To accommodate this it has been 
decided that python packages will be deployed using eggs. In order for this
to work it is necessary to install the setuptools package.

Setup Local Python Configuration
================================
Create a file ~/.pydistutils.cfg with the following content:
>>> Start on next line
[install]
install_lib = ~/lib/python
install_scripts = ~/bin
<<< End on previous line

cd ~
mkdir -p lib/python
mkdir -p bin

Add ~/bin to $PATH
Add ~/lib/python to $PYTHONPATH

The above steps make installs go to the specified directories which do
not require root permission.

Express install (for the very brave - takes a long time)
===============
The following will (hopefully) automatically install setuptools, recursivebuild,
Tools, 3rdParty and Code. If you don't want to do it all at once then
skip this and move to the next step.
cd svnCONRAD
python setup.py -q install

Installing setuptools (http://peak.telecommunity.com/DevCenter/setuptools)
=====================
Python needs to be configured to work with setuptools. To do this,
execute the following commands:
cd svnCONRAD/Tools/Dev/setuptools
python ez_setup.py

Installing recursivebuild
=========================
cd svnCONRAD/Tools/Dev/recursivebuild
python setup.py install

This will install this tiny module in the standard python path, so that it can
hierarchically build the repository in the many setup.py scripts.

Installing Tools
================
cd svnCONRAD/Tools
python setup.py -q install

Installing 3rdParty
==============================
cd svnCONRAD/3rdParty
python setup.py -q install

Install Code
============
cd svnCONRAD/Code
python setup.py -q install

Additional repositories (eg svnComputing/AutoBuild)
=======================
Build first Tools, then 3rdParty and finally the Code directory. In
each case the command is:
python setup.py -q install