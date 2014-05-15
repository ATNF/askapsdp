#!/usr/bin/env python
import sys
import os
import threading
import time

# pylint: disable-msg=E0611
from nose.tools import assert_equals, assert_not_equals, assert_items_equal
from askap.iceutils import IceSession
from askap.iceutils.typedvaluepublisher import (TypedValuePublisher,
                                                TimeTaggedTypedValuePublisher)
from askap.iceutils.typedvaluesubscriber import TypedValueSubscriber
from askap.interfaces.datapublisher import (ITypedValueMapPublisher,
                                            ITimeTaggedTypedValueMapPublisher)

from askap.iceutils.monitorpublisher import Monitoring, get_monitor, MonitorData
from askap.iceutils.typedvalues import dict_mapper

# pylint: disable-msg=W0232
class SubscriberImpl(ITypedValueMapPublisher):
    def __init__(self, event=None):
        self._event = event

    # pylint: disable-msg=W0613,W0603,R0201
    def publish(self, data, current=None):
        self._event.set()

# pylint: disable-msg=W0232
class TTSubscriberImpl(ITimeTaggedTypedValueMapPublisher):
    def __init__(self, event=None, data=None, length=0):
        self._event = event
        self.data = data
        self.length = length

    # pylint: disable-msg=W0613,W0603,R0201
    def publish(self, data, current=None):
        if self.data is not None:
            ddata = dict_mapper(data.data)
            self.data.append(ddata)
            if len(self.data) == self.length:
                self._event.set()
        else:
            self._event.set()


class TestPubSub(object):
    def __init__(self):
        self.logger = None
        self.topic = "testtopic"
        self.isession = None

    def setup(self):
        os.environ["ICE_CONFIG"] = 'ice.cfg'
        os.chdir('functests')
        self.isession = IceSession('applications.txt', cleanup=True)
        try:
            self.isession.start()
        except Exception as ex:
            self.teardown()
            raise

    def teardown(self):
        os.chdir('..')
        self.isession.terminate()

    def test_typedvalue_publish(self):
        block = threading.Event()
        self.publisher = TypedValuePublisher(
            topic=self.topic,
            communicator=self.isession.communicator
            )
        self.subscriber = TypedValueSubscriber(
            topic=self.topic, communicator=self.isession.communicator,
            impl_cls=SubscriberImpl, impl_kwargs={'event': block}
            )
        self.publisher.publish({'test': True})
        nottimedout = block.wait(3.0)
        #python2.6
        nottimedout = block.is_set()
        assert_equals(nottimedout, True)

    def test_timetaggedtypedvalue_publish(self):
        block = threading.Event()
        self.publisher = TimeTaggedTypedValuePublisher(
            topic=self.topic,
            communicator=self.isession.communicator
            )
        self.subscriber = TypedValueSubscriber(
            topic=self.topic, communicator=self.isession.communicator,
            impl_cls=TTSubscriberImpl, impl_kwargs={'event': block}
            )
        self.publisher.publish({'test': True}, 12345L)
        nottimedout = block.wait(3.0)
        nottimedout = block.is_set()
        assert_equals(nottimedout, True)

        

class TestMonitors:
    def __init__(self):
        self.logger = None
        self.topic = "testtopic"
        self.isession = None

    def setup(self):
        os.environ["ICE_CONFIG"] = 'ice.cfg'
        os.chdir('functests')
        self.isession = IceSession('applications.txt', cleanup=True)
        try:
            self.isession.start()
        except Exception as ex:
            self.teardown()
            raise

    def teardown(self):
        os.chdir('..')
        self.isession.terminate()

    def test_monitor_send(self):
        data = []
        block = threading.Event()
        with Monitoring(topic=self.topic, 
                           communicator=self.isession.communicator) as mon:
            subscriber = TypedValueSubscriber(
                topic=self.topic, communicator=self.isession.communicator,
                impl_cls=TTSubscriberImpl, impl_kwargs={'event': block,
                                                        'data': data,
                                                        'length': 10}
                )
            outd = []
            for i in range(10):
                mon.add_points({'foo': i})
            nottimedout = block.wait(5.0)
            assert_equals(nottimedout, True)

    def test_monitor_send(self):
        data = []
        block = threading.Event()
        with Monitoring(topic=self.topic, 
                           communicator=self.isession.communicator) as mon:
            subscriber = TypedValueSubscriber(
                topic=self.topic, communicator=self.isession.communicator,
                impl_cls=TTSubscriberImpl, impl_kwargs={'event': block,
                                                        'data': data,
                                                        'length': 10}
                )
            outd = []
            for i in range(10):
                mon.add_points({'foo{}'.format(i) : i})
            nottimedout = block.wait(5.0)
            assert_equals(nottimedout, True)

    def test_monitor_data(self):
        data = []
        block = threading.Event()
        mon = get_monitor(self.topic, self.isession.communicator)
        with mon:
            subscriber = TypedValueSubscriber(
                topic=self.topic, communicator=self.isession.communicator,
                impl_cls=TTSubscriberImpl, impl_kwargs={'event': block,
                                                        'data': data,
                                                        'length': 10}
                )
            outd = []
            for i in range(10):
                md = MonitorData()
                md['foo{}'.format(i)] = i
                outd.append({'foo{}'.format(i): i})
                md.publish()
            nottimedout = block.wait(5.0)
        assert_equals(nottimedout, True)
        for i in range(10):            
            assert_equals(data[i], outd[i]) 

        
