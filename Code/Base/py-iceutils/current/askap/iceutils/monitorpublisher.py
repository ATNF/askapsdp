# Copyright (c) 2014 CSIRO
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
__all__ = ["get_monitor", "MonitorData"]

import sys
import os
from threading import Lock, Event, Thread
import Ice

from askap import logging
from askap.time import bat_now

from .typedvaluepublisher import TimeTaggedTypedValuePublisher

logger = logging.getLogger(__name__)

_MONITOR_SINGLETON = None


def get_monitor(topic=None, icecomm=None):
    global _MONITOR_SINGLETON
    if topic is None:
        if _MONITOR_SINGLETON is None:
            raise RuntimeError("Monitor not yet configured")
        return _MONITOR_SINGLETON
    if _MONITOR_SINGLETON is not None:
        if _MONITOR_SINGLETON.topic != topic:
            raise KeyError("Monitoring already set up under topic '{}'"
                           .format(_MONITOR_SINGLETON.topic))
        return _MONITOR_SINGLETON
    if icecomm is None:
        iceenv = os.environ["ICE_CONFIG"]
        icecomm = Ice.initialize(sys.argv)
    _MONITOR_SINGLETON = Monitoring(topic, icecomm)
    return _MONITOR_SINGLETON


class MonitorData(dict):
    """ :class:`dict` class adding :meth:`send` to send the dictionary
    to :class:`Monitoring` singelton.
    """
    def __init__(self, *args, **kwargs):
        dict.__init__(self, *args, **kwargs)

    def publish(self):
        mon = get_monitor()
        mon.add_points(self)


class Monitoring(object):
    def __init__(self, topic, communicator):
        self._mutex = Lock()
        self._buffer = []
        self._stop = Event()
        self.notify = Event()
        self.topic = topic
        self.error = None
        self._publisher = TimeTaggedTypedValuePublisher(topic,
                                                        communicator)
        self._max_size = 1000
        self._thread = Thread(target=self.sender)
        self._thread.daemon = True
        self._thread.start()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.shutdown()
        if not exc_type and self.error is not None:
            logger.fatal(self.error)
            raise RuntimeError(self.error)

    def shutdown(self):
        self._stop.set()
        self._thread.join()
        self._thread = None

    def add_points(self, points, timetag=None):
        """Add a :class:`dict` of point(s) to the buffer."""
        if self.error is not None:
            logger.fatal(self.error)
            raise RuntimeError(self.error)
        timestamp = timetag or bat_now()
        with self._mutex:
            if len(self._buffer) > self._max_size:
                self._buffer.pop(0)
            self._buffer.append((points, timestamp))
        self.notify.set()
        self.notify.clear()
        
    def sender(self):
        while not self._stop.is_set():
            retries = 0
            self.notify.wait(1)
            while len(self._buffer) > 0:
                if self._stop.is_set():
                    return
                t = None
                point = None
                with self._mutex:
                    point, t = self._buffer.pop(0)
                try:
                    self._publisher.publish(point, t)
                except Exception as ex:
                    print ex
                    # reinsert 
                    if retries > 5:
                        self.error = "Repeatedly unable to send monitoring data"
                        self._stop.set()
                        return
                    logger.warn("Couldn't publish on topic '{}'"\
                                     .format(self.topic))
                    retries += 1
                    with self._mutex:
                        self._buffer.append((point, t))
                    


        
