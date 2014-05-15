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
__all__ = ["Server"]
import sys
import os
import time
import Ice

# pylint: disable-msg=W0611
from askap.slice import FCMService
import askap.interfaces.fcm

from askap.parset import parset_to_dict

from askap import logging
logger = logging.getLogger(__name__)

class TimeoutError(Exception):
    """An exception raised when call timed out"""
    pass

class Server(object):
    """A class to abstract an ice application (server)"""
    def __init__(self, comm, fcmkey='', retries=-1):
        self.parameters = None
        self._comm = comm
        self._adapter = None
        self._services = []
        self._retries = retries
        self.service_key = fcmkey
        self.logger = logger
        
    def set_retries(self, retries):
        self._retries = retries

    def get_config(self):
        key = "--config="
        for arg in sys.argv:
            if arg.startswith(key):
                k, v = arg.split("=")
                p = os.path.expanduser(os.path.expandvars(v))
                self.parameters = parset_to_dict(open(p).read())
                self.logger.info("Initialized from local config '%s'" % p)
                return
        self._config_from_fcm()

    def _config_from_fcm(self):
        pmap = {}
        prxy = self._comm.stringToProxy("FCMService@FCMAdapter")
        if not prxy:
            raise RuntimeError("Invalid Proxy for FCMService")
        fcm = self.wait_for_service("FCM",
                           askap.interfaces.fcm.IFCMServicePrx.checkedCast,
                           prxy)
        self.parameters = fcm.get(-1, self.service_key)
        self.logger.info("Initialized from fcm")

    def get_parameter(self, key, default=None):
        return self.parameters.get(".".join((self.service_key, key)), default)

    def wait_for_service(self, servicename, callback, *args):
        retval = None
        delay = 5.0
        count = 0
        registry = False
        while not registry:
            try:
                retval = callback(*args)
                registry = True
            except (Ice.ConnectionRefusedException,
                    Ice.NoEndpointException,
                    Ice.NotRegisteredException,
                    Ice.ConnectFailedException,
                    Ice.DNSException) as ex:
                if self._retries > -1 and self._retries == count:
                    msg = "Couldn't connect to {0}".format(servicename)
                    self.logger.error(msg)
                    raise TimeoutError(msg)
                if count < 10:
                    print >>sys.stderr, "Waiting for", servicename
                if count == 10:
                    print >>sys.stderr, "Repeated 10+ times"
                    self.logger.warn("Waiting for {0}".format(servicename))
                registry = False
                count += 1
                time.sleep(delay)
        if registry:
            self.logger.info("Connected to {0}".format(servicename))
            print >>sys.stderr, servicename, "found"
        return retval

    def run(self):
        adname = self.__class__.__name__+"Adapter"
        self._adapter = self._comm.createObjectAdapter(adname)
        self.get_config()

        ## implement this method in derived class
        self.initialize_services()

        for service in self._services:
            self._adapter.add(service['value'],
                              self._comm.stringToIdentity(service['name']))

        self.wait_for_service("registry", self._adapter.activate)
            
        
        self._comm.waitForShutdown()

    def add_service(self, name, value):
        self._services.append({'name': name, 'value': value})

    def initialize_services(self):
        """Implement in sub-class"""
        pass
