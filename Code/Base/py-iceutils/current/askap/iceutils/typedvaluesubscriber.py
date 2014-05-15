# @copyright (c) 2013 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
__all__ = ["TypedValueSubscriber"]

import time
import Ice, IceStorm

# pylint: disable-msg=W0611
from askap.slice import TypedValues

# ice doesn't agree with pylint
# pylint: disable-msg=E0611
import askap.interfaces as iceint
from askap.interfaces.datapublisher import (ITimeTaggedTypedValueMapPublisher,)

class DefaultTypeValueMapImpl(ITimeTaggedTypedValueMapPublisher):
    def __init__(self, **kwargs):
        self._kwargs = kwargs

    def publish(self, data, current=None):
        print data

class TypedValueSubscriber(object):
    """IceStorm subscriber to TypedValue based topics"""
    def __init__(self, topic='metadata', communicator=None,
                 impl_cls=None, impl_kwargs=None):
        if communicator:
            self.ice = communicator
        else:
            host = 'localhost'
            port = 4061
            self.ice = self._setup_communicator(host, port)

        self._impl = impl_cls
        self._impl_kwargs = impl_kwargs or {} 
        self.prxy = None
        self.manager = None
        self._topic = topic
        self._setup_icestorm()

    def _setup_communicator(self, host, port):
        init = Ice.InitializationData()
        init.properties = Ice.createProperties()
        loc = "IceGrid/Locator:tcp -h "+ host + " -p " + str(port)
        init.properties.setProperty('Ice.Default.Locator', loc)
        init.properties.setProperty('Ice.IPv6', '0')
        return Ice.initialize(init)

    def _setup_icestorm(self):
        """Create the IceStorm connection and subscribe to the logger topic.
        """
        if not self.manager:
            prxstr = self.ice.stringToProxy(
                'IceStorm/TopicManager@IceStorm.TopicManager')
            try:
                self.manager = IceStorm.TopicManagerPrx.checkedCast(prxstr)
            except (Ice.LocalException, Exception) as ex:
                self.manager = None
                raise
        try:
            self.topic = self.manager.retrieve(self._topic)
        except IceStorm.NoSuchTopic:
            try:
                self.topic = self.manager.create(self._topic)
            except IceStorm.TopicExists:
                self.topic = self.manager.retrieve(topicname)

        aname = self.__class__.__name__+self._topic.capitalize()+"Adapter"
        self.adapter = \
            self.ice.createObjectAdapterWithEndpoints(aname, "tcp")

        cls = self._impl or DefaultTypeValueMapImpl
        self.subscriber = \
            self.adapter.addWithUUID(cls(**self._impl_kwargs)).ice_oneway()
        qos = {}
        try:
            self.topic.subscribeAndGetPublisher(qos, self.subscriber)
        except IceStorm.AlreadySubscribed:
            self.topic.unsubscribe(self.subscriber)
            self.topic.subscribeAndGetPublisher(qos, self.subscriber)
        self.adapter.activate()

    def wait(self):
        self.ice.waitForShutdown()
        self.topic.unsubscribe(self.subscriber)

if __name__ == "__main__":

    msub = TypedValueSubscriber()
    msub.wait()
