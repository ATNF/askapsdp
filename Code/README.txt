Building Python Projects
========================

A core assumption of the build process is that many different baseline
versions of the same libraries will exist. To accommodate this it has been 
decided that python packages will be deployed using eggs. In order for this
to work it is necessary to install the setuptools package.

Installing setuptools (http://peak.telecommunity.com/DevCenter/setuptools)
=====================
Python needs to be configured work with setuptools. To do this go to
ThirdPartyLibraries/Python/setuptools and run the following command
sudo python ez_setup.py

Setup Local Python Configuration
================================
Now we need python to be configured to install eggs into ~/lib/python2.4
which does not require root access. To do this look at
http://peak.telecommunity.com/DevCenter/EasyInstall and go about halfway
down to Administrator Installation or Creating a "Virtual" Python.

The above configuration will result in ~/lib/python2.4 being used as the
site-packages location when you execute setup.py install.

PYTHONPATH
==========
You must add ~/lib/python2.4 to your PYTHONPATH

You must now build the recursivebuild python module. To
do that execute the following commands:
cd recursivebuild
su
[password]
python setup.py install
exit
cd ..

This will install the tiny module in the standard python path so that it can be
in the many setup.py scripts to hierarchically build the repository.
