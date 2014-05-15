# Copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
"""
This module contains wrappers to start up and administer IceGrid.
"""
from __future__ import with_statement
import os
import sys
import shutil
import subprocess
import time
import re
import Ice
import IceGrid
import threading

class IceGridSession(object):
    def __init__(self, configfile='config.icegrid', stddir='debug_logs'):
        """Control object of IceGrid. It simplifies calls to administer IceGrid.
        It can be used to startup, shutdown and add applications from xml
        definiions.

        :param configfile: The IceGrid configuration file.
        :param stddir: Directory to write stdout, stderr files

        """
        self.communicator = None
        self._user = None
        self._passwd = None
        self._admin = None
        self._cleanup = True
        self.debug = False
        self.stddir = stddir
        self._clean_targets = []
        self._kathread = None
        if not os.path.exists(configfile):
            raise IOError("'%s' doesn't exist" % configfile)
        self._cfg  = configfile
        self.init_ice()

    def __del__(self):
        self.shutdown()

    def _kill_existing(self, name):
        """Kill an existing registry if running"""
        cmd = 'icegridadmin --Ice.Config=%s -u %s -p %s -e'\
              '"registry shutdown %s"' % (self._cfg, self._user, self._passwd,
                                          name)
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             shell=True)
        o,e = p.communicate()
        if p.returncode == 0:
            print sys.stderr, "Warning: Found existing icegridnode process. "\
                              "Shutting it down."
            time.sleep(1)

    def init_ice(self):
        init = Ice.InitializationData()
        init.properties = Ice.createProperties()
        init.properties.load(self._cfg)
        regname = init.properties.getProperty('IceGrid.Registry.ReplicaName')
        self._kill_existing(regname)
        if self._cleanup:
            for f in [init.properties.getProperty('Ice.StdOut'),
                      init.properties.getProperty('Ice.StdErr')]:
                if f and os.path.exists(f):
                    os.remove(f)
            dirs = [init.properties.getProperty('IceGrid.Registry.Data')]
            dirs.append(init.properties.getProperty('IceGrid.Node.Data'))
            for d in dirs:
                d = os.path.split(d)[0]
                if os.path.exists(d):
                    shutil.rmtree(d)
        for d in dirs:
            if not os.path.exists(d):
                os.makedirs(d)
        self._user = init.properties.getProperty('IceGridAdmin.Username')
        self._passwd = init.properties.getProperty('IceGridAdmin.Password')
        self.communicator = Ice.initialize(init)

    def startup(self, timeout=10):
        """Start up IceGrid and create and Administration Session.

        :param timeout: The period in seconds to wait for icegridnode to be
                        initialised before failing.

        """
        if self._admin:
            raise RuntimeError("Admin has already been started")
        cmd = 'icegridnode --Ice.Config=%s > /dev/null 2>&1 &' % self._cfg
        retcode = subprocess.call(cmd, shell=True)
        notup = True
        prx = self.communicator.stringToProxy('IceGrid/Registry')
        registry = None
        while notup and timeout != 0:
            try:
                registry = IceGrid.RegistryPrx.checkedCast(prx)
            except Exception,e:
                #print e
                timeout -= 1
                time.sleep(1)
                continue
            notup = False
        if timeout == 0:
            raise RuntimeError("Waiting for icegrid timed out")
        if not registry:
            raise RuntimeError("Registry not found")
        session = registry.createAdminSession(self._user, self._passwd)
        self._kathread = _KeepAliveThread(session)
        self._kathread.start()
        self._admin = session.getAdmin()

    def shutdown(self):
        """Shutdown IceGrid. This will not close the applications beforehand,
        so the state wil remain in the 'data' directory.
        """
        for tgt in self._clean_targets:
            if os.path.exists(tgt):
                os.remove(tgt)
        self._clean_targets = []
        if self._kathread is not None:
            self._kathread.stop()
            self._kathread = None
        if self._admin is not None:
            # shutdown the registry - and therefore IceGrid
            self._admin.shutdown()
            # Need to wait here so the next call works.
            self._wait_for_shutdown()
            self._admin = None

    def _wait_for_shutdown(self):
        timeout = 10
        while timeout != 0:
            try:
                prx = self.communicator.stringToProxy('IceGrid/Registry')
                registry = IceGrid.RegistryPrx.checkedCast(prx)
            except:
                break
            timeout -= 1
            time.sleep(1)


    def add_application(self, appxml, config=None):
        """Add an application to IceGrid given and xml definition (file)

        :param appxml: The application description file

        """
        if not self._admin:
            raise TypeError("Admin is not initialised")
        if not os.path.exists(appxml):
            raise IOError("xml application definition file doesn't exist")
        # Can't use xml to add application as IceGrid::FileParser is not
        # implemented in python
        if config is None:
            config = self._cfg
        else:
            if not os.path.exists(conf):
                raise IOError("File '%s' doesn't exist" % config)
        tmpfiles = []
        if self.debug and appxml.find("icestorm") < 0:
            tmpfiles  = self.redirect_std(appxml)
            appxml = tmpfiles[-1]
            self._clean_targets += tmpfiles
        cmd = 'icegridadmin --Ice.Config=%s -u %s -p %s -e "application add %s"' % (config, self._user, self._passwd, appxml)
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             shell=True)
        o,e = p.communicate()
        if p.returncode != 0:
            raise RuntimeError(e)

    def redirect_std(self, xmlfile):
        #stddir = "debug_logs"
        if not os.path.exists(self.stddir):
            os.makedirs(self.stddir)
        filenoext = os.path.splitext(xmlfile)[0]
        rx = re.compile('(exe=")(.+?)(")', re.VERBOSE)
        lines = open(xmlfile).readlines()
        filebase = os.path.basename(filenoext)
        appname = None
        tmpxmlname = filenoext + "_tmp" + ".xml"
        with open(tmpxmlname, 'w') as tmpxml:
            for line in lines:
                match = rx.search(line)
                if match:
                    appname = match.group(2)
                    line = rx.sub('\g<1>./%s_tmp.sh\g<3>' % filebase, line)
                tmpxml.write(line)
        tmpname = filebase+ "_tmp.sh"
        stdpath = os.path.join(self.stddir, filebase)
        txt ="""#!/bin/bash
%s "$@" > %s.stdout 2> %s.stderr
""" % (appname, stdpath, stdpath)
        with open(tmpname, 'w') as tmpexe:
            tmpexe.write(txt)
        os.chmod(tmpname, 0755)
        return [os.path.abspath(tmpname), os.path.abspath(tmpxmlname)]

    def get_application_names(self):
        """Return the names of all appications in this Node"""
        return self._admin.getAllApplicationNames()

    def remove_application(self, appname):
        """Remove an application given its name"""
        if not self._admin:
            raise TypeError("Admin is not initialised")
        self._admin.removeApplication(appname)

    def add_icestorm(self):
        if not self._admin:
            raise TypeError("Admin is not initialised")
        dbg = self.debug
        self.debug = False
        self.add_application("icestorm.xml")
        self.debug = dbg


class _KeepAliveThread(threading.Thread):
    def __init__(self, session, interval=5):
        self._session = session
        self._interval = interval
        self._stopped = threading.Event()
        threading.Thread.__init__(self)

    def run(self):
        while not self._stopped.is_set():
            time.sleep(self._interval)
            try:
                self._session.keepAlive()
            except:
                pass

    def stop(self):
        self._stopped.set()

if __name__ == "__main__":
    igs = IceGridSession("config.icegrid")
    igs.startup()
    igs.add_icestorm()
    print igs.get_application_names()
    igs.shutdown()
