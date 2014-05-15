# Copyright (c) 2013 CSIRO
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
__all__ = ["TypedValuePublisher", "TimeTaggedTypedValuePublisher"]

import Ice, IceStorm

from . import get_service_object, get_communicator
from .typedvalues import typed_mapper


# ice doesn't agree with pylint
# pylint: disable-msg=E0611
import askap.interfaces as iceint
from askap.interfaces.datapublisher import (
    ITimeTaggedTypedValueMapPublisher,
    ITimeTaggedTypedValueMapPublisherPrx,
    ITypedValueMapPublisherPrx)


class TypedValuePublisher(object):
    """IceStorm publisher for TOS metadata"""
    def __init__(self, topic='metadata', communicator=None, host='localhost',
                 port=4061, timetagged=False):
        if communicator:
            self.ice = communicator
        else:
            self.ice = get_communicator(host, port)
        self.timetagged = timetagged
        self.prxy = None
        self.manager = None
        self._topic = topic
        self._setup_icestorm()

    def _setup_icestorm(self):
        """Create the IceStorm connection and subscribe to the logger topic.
        """

        if not self.manager:
            self.manager = get_service_object(self.ice,
                              'IceStorm/TopicManager@IceStorm.TopicManager',
                                          IceStorm.TopicManagerPrx)
        try:
            topic = self.manager.retrieve(self._topic)
        except IceStorm.NoSuchTopic:
            try:
                topic = self.manager.create(self._topic)
            except IceStorm.TopicExists:
                return

        publisher = topic.getPublisher()
        publisher = publisher.ice_oneway()
        prx = ITypedValueMapPublisherPrx
        if self.timetagged:
            prx = ITimeTaggedTypedValueMapPublisherPrx
        self.prxy = \
            prx.uncheckedCast(publisher)

    def publish(self, data, timestamp=None):
        """Publish a TimeTaggedTypedValueMap for the given `timestamp` and 
        :class:`dict` `data` under the given `topic` (default *metadata*).
        """

        if not self.manager or not self.prxy:
            self._setup_icestorm()
            if not self.manager or not self.prxy:
                return
        metadata = typed_mapper(data, timestamp)
        self.prxy.publish(metadata)


class TimeTaggedTypedValuePublisher(TypedValuePublisher):
    def __init__(self, topic='metadata', communicator=None, host='localhost',
                 port=4061):
        TypedValuePublisher.__init__(self, topic, communicator, host,
                                     port, True)
        

if __name__ == "__main__":
    import time
    testdict = {'on_source': True,
                'scan_id' : -1, 
                'pointing_radec': (0.0, -1.0, 'J2000'),
                'test_value': ["S", "T"],
                'test': 1+1j,
                'test2': 1.0,
                }
    bat = 1000L
    mpub = TimeTaggedTypedValuePublisher()
    for i in range(3L):
        mpub.publish(testdict, bat+i*5)
        if i < 2:
            time.sleep(5)


