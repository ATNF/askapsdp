# Copyright (c) 2009-2013 CSIRO
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
"""Utilities to simplify ZeroC Ice application creation and deployment in the 
ASKAPsoft environment"""
__all__ = ["Server", "IceSession", "get_service_object", "get_communicator",
           "IceService"]
import sys
import os
import time
import Ice
from .icesession import IceSession


class IceService(object):
    """Helper class to access Ice and get service proxies."""
    def __init__(self):
        self._init_ice()

    def _init_ice(self):
        if 'ICE_CONFIG' not in os.environ:
            raise OSError("ICE_CONFIG not defined")
        self.ice = Ice.initialize(sys.argv)        

    def get_service(self, name, prx):
        """get the Ice service object"""
        return get_service_object(self.ice, name, prx, 1)



def get_service_object(communicator, name, prx, retries=25):
    """Try to obtain a service object (via checkedCast) with a maximum number
    of retries.

    Example::

        get_service_object(mycommunicator,
                "SchedulingBlockService@DataServiceAdapter",
                askap.interfaces.schedblock.ISchedulingBlockServicePrx)
    """
    obj = None
    p = communicator.stringToProxy(name)
    if not p:
        raise RuntimeError("Invalid Proxy")
    i = 0
    while i < retries:
        try:
            obj = prx.checkedCast(p)
            break
        except (Ice.NotRegisteredException, 
                Ice.ConnectionRefusedException,
                Ice.NoEndpointException,
                ) as ex:
            print i, ":",ex
#            pass
        time.sleep(0.5)
        i +=1
    if obj is None:
        raise RuntimeError("Registry request timed out")
    return obj


def get_communicator(host, port):
    """Return a Ice communicator instance"""
    init = Ice.InitializationData()
    init.properties = Ice.createProperties()
    loc = "IceGrid/Locator:tcp -h "+ host + " -p " + str(port)
    init.properties.setProperty('Ice.Default.Locator', loc)
    init.properties.setProperty('Ice.IPv6', '0')
    return Ice.initialize(init)

# prevent circular imports
from .server import Server
