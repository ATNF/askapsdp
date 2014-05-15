#!/usr/bin/env python
# Copyright (c) 2012 CSIRO
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
__all__ = ["IceSession"]
import sys
import os
import time
import signal
import subprocess
import threading
import shutil
import Ice

class IceSession(object):
    """
    Deploy a list of ice application with a registry.
    This is a *SIGTERM* interruptable process which will stop all applications
    gracefully.

    :param app_file: an optional text file with command lines
    :param cleanup: optional flag to indicate cleanup of ice meta-data

    *ICE_CONFIG* needs to be defined in the environment.
    Applications can be added programatically via :meth:`.add_app` or via a file
    given at contruction or via :meth:`from_file`. The file should contain one 
    line per executable with arguments e.g.
    
    .. code-block:: sh
         
         icebox
         fcm.py --log-config=askap.pylog_cfg --config=fcm_init.parset
    

    The class provides a context manager, i.e. it can and shoudl be used with a
    `with` statement::

        with IceSession() as iceenv:
            iceenv.start()
            do_stuff()
     
        # all process will terminate here

    """
    def __init__(self, app_file=None, cleanup=False):
        if not "ICE_CONFIG" in os.environ:
            raise OSError("Environment variable ICE_CONFIG not defined")
        self.apps = []
        """The list of applications to create subprocesses for"""
        self.processes = []
        """The list of subprocesses"""
        self._started = False
        self._clean_dirs = []
        self.cleanup = cleanup
        """Clean up ice meta-data directories on terminate"""
        self.communicator = None
        """An :obj:`Ice.CommunicatorI` instance given *ICE_CONFIG*"""
        self.wake_up = threading.Event()
        """Wake up :meth:`.wait`"""
        signal.signal(signal.SIGTERM, self.terminate)
        self._init_communicator()
        if self.registry_running():
            raise RuntimeError("icegridregistry with same config "
                               "already running")
        self.add_app("icegridregistry")
        self.from_file(app_file)

    def __enter__(self):
        return self
    
    def __exit__(self, ttype, value, traceback):
        self.terminate()

    def _init_communicator(self):
        self.communicator = Ice.initialize(sys.argv)
        props = self.communicator.getProperties()
        dirs = [props.getProperty('IceGrid.Registry.Data'),
                props.getProperty('Freeze.DbEnv.IceStorm.DbHome')]
        for d in dirs:
            if not d:
                continue
            if os.path.exists(d) and self.cleanup:
                shutil.rmtree(d)                
            if not os.path.exists(d):
                print "creating ice metadata directory", d
                os.makedirs(d)
                self._clean_dirs.append(d)

    def terminate(self, signum=None, frame=None):
        """Terminate all application processes in reverse order"""
        for proc in self.processes[::-1]:            
            try:
                proc.terminate()
                proc.wait()
                print >>sys.stdout, "SIGTERM", proc.application
            except Exception as ex:
                print >>sys.stderr,ex 
                
        self.processes = []
        self.wake_up.set()
        self._started = False

    def from_file(self, filename=None):
        """Load application list from file provided in :attr:`sys.argv`.
        """
        if filename is None:
            return
        with open(filename, "r") as f:
            for line in f:
                line = line.strip()
                if len(line) > 0 and not line.startswith("#"):
                    elem = line.split()
                    app = elem[0]
                    args = []
                    if len(elem) > 1:
                        args = elem[1:]
                    self.add_app(app, args)

    def add_app(self, app, args=None):
        """Add an application with optional arguments. This doesn't run the
        application.
        
        :param str app: the application name
        :param list args: the optional arguments for the application
        
        """
        if args is None:
            args = []
        self.apps.append((app, args))

    def run_app(self, application, args):
        """Run the application in a background process and add to 
        :attr:`.processes`"""
        lfile = application+".log"
        cmd = [application]+args
        with open(lfile,"w") as logfile:
            proc = subprocess.Popen(cmd, shell=False,
                                    stderr=logfile, stdout=logfile)
            self.processes.append(proc)
            proc.application = application
            proc.log = lfile
            if application == "icegridregistry":
                self.wait_for_registry()
            if proc.poll() is not None:
                print >>sys.stderr, proc.application, "failed:"
                print >>sys.stderr, open(proc.log, 'r').read()
                raise RuntimeError("Application '%s' failed on start"
                                   % application)

            
    def registry_running(self):
        """Check wether an icegridregistry is running"""
        try:
            self.communicator.getDefaultLocator().ice_ping()
            return True
        except Ice.ConnectionRefusedException as ex:
            return False


    def wait_for_registry(self, timeout=10):
        """Block until the ice registry is up and time out after `timeout` 
        seconds.
        """
        n = timeout/0.1
#        t0 = time.time()
        i = 0
        connected = False
        while i < n:
            try:
                self.communicator.getDefaultLocator().ice_ping()
                connected = True
                break
            except Ice.ConnectionRefusedException as ex:
                pass
            time.sleep(0.1)
            i += 1
        if not connected:
            raise RuntimeError("Waiting for icegridregistry timed out")
#        print >>sys.stderr,i, time.time() - t0
        

    def wait(self):
        """Block the main process. This is interruptable through a 
        :class:`KeyboardInterrupt` (Ctrl-C)."""
        self.wake_up.clear()
        if self.processes:
            # periodically check if a subprocess had died
            while True:                
                for proc in self.processes:
                    if proc.poll() is not None:
                        raise RuntimeError("Process %s died" 
                                           % proc.application) 
                self.wake_up.wait(1)        
                if self.wake_up.is_set():
                    return
    
    def start(self):
        """Start all applications, e.g call :meth:`run_app` on all 
        :attr:`apps`"""
        if self._started:
            return
        try:
            for application, args in self.apps:
                print "Starting", application, "...",
                sys.stdout.flush()
                self.run_app(application, args)
                print "done."
            self._started = True
        except KeyboardInterrupt:
            # need to wait here otherwise the processes won't terminate???
            time.sleep(1)
            self.terminate()

if __name__ == "__main__":
    fname = None
    if len(sys.argv) == 2:
        fname = sys.argv[-1]
    
    with IceSession(fname, cleanup=True) as isess:
        isess.start()
        isess.wait()
