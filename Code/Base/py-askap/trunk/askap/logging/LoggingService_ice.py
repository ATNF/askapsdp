# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Ice version 3.3.0
# Generated from file `LoggingService.ice'

import Ice, IcePy, __builtin__

# Start of module askap
_M_askap = Ice.openModule('askap')
__name__ = 'askap'

# Start of module askap.logging
_M_askap.logging = Ice.openModule('askap.logging')
__name__ = 'askap.logging'

# Start of module askap.logging.interfaces
_M_askap.logging.interfaces = Ice.openModule('askap.logging.interfaces')
__name__ = 'askap.logging.interfaces'

if not _M_askap.logging.interfaces.__dict__.has_key('LogEvent'):
    _M_askap.logging.interfaces.LogEvent = Ice.createTempClass()
    class LogEvent(object):
        def __init__(self, origin='', created=0.0, level='', message=''):
            self.origin = origin
            self.created = created
            self.level = level
            self.message = message

        def __hash__(self):
            _h = 0
            _h = 5 * _h + __builtin__.hash(self.origin)
            _h = 5 * _h + __builtin__.hash(self.created)
            _h = 5 * _h + __builtin__.hash(self.level)
            _h = 5 * _h + __builtin__.hash(self.message)
            return _h % 0x7fffffff

        def __cmp__(self, other):
            if other == None:
                return 1
            if self.origin < other.origin:
                return -1
            elif self.origin > other.origin:
                return 1
            if self.created < other.created:
                return -1
            elif self.created > other.created:
                return 1
            if self.level < other.level:
                return -1
            elif self.level > other.level:
                return 1
            if self.message < other.message:
                return -1
            elif self.message > other.message:
                return 1
            return 0

        def __str__(self):
            return IcePy.stringify(self, _M_askap.logging.interfaces._t_LogEvent)

        __repr__ = __str__

    _M_askap.logging.interfaces._t_LogEvent = IcePy.defineStruct('::askap::logging::interfaces::LogEvent', LogEvent, (), (
        ('origin', (), IcePy._t_string),
        ('created', (), IcePy._t_double),
        ('level', (), IcePy._t_string),
        ('message', (), IcePy._t_string)
    ))

    _M_askap.logging.interfaces.LogEvent = LogEvent
    del LogEvent

if not _M_askap.logging.interfaces.__dict__.has_key('ILogger'):
    _M_askap.logging.interfaces.ILogger = Ice.createTempClass()
    class ILogger(Ice.Object):
        def __init__(self):
            if __builtin__.type(self) == _M_askap.logging.interfaces.ILogger:
                raise RuntimeError('askap.logging.interfaces.ILogger is an abstract class')

        def ice_ids(self, current=None):
            return ('::Ice::Object', '::askap::logging::interfaces::ILogger')

        def ice_id(self, current=None):
            return '::askap::logging::interfaces::ILogger'

        def ice_staticId():
            return '::askap::logging::interfaces::ILogger'
        ice_staticId = staticmethod(ice_staticId)

        #
        # Operation signatures.
        #
        # def send(self, event, current=None):

        def __str__(self):
            return IcePy.stringify(self, _M_askap.logging.interfaces._t_ILogger)

        __repr__ = __str__

    _M_askap.logging.interfaces.ILoggerPrx = Ice.createTempClass()
    class ILoggerPrx(Ice.ObjectPrx):

        def send(self, event, _ctx=None):
            return _M_askap.logging.interfaces.ILogger._op_send.invoke(self, ((event, ), _ctx))

        def checkedCast(proxy, facetOrCtx=None, _ctx=None):
            return _M_askap.logging.interfaces.ILoggerPrx.ice_checkedCast(proxy, '::askap::logging::interfaces::ILogger', facetOrCtx, _ctx)
        checkedCast = staticmethod(checkedCast)

        def uncheckedCast(proxy, facet=None):
            return _M_askap.logging.interfaces.ILoggerPrx.ice_uncheckedCast(proxy, facet)
        uncheckedCast = staticmethod(uncheckedCast)

    _M_askap.logging.interfaces._t_ILoggerPrx = IcePy.defineProxy('::askap::logging::interfaces::ILogger', ILoggerPrx)

    _M_askap.logging.interfaces._t_ILogger = IcePy.defineClass('::askap::logging::interfaces::ILogger', ILogger, (), True, None, (), ())
    ILogger.ice_type = _M_askap.logging.interfaces._t_ILogger

    ILogger._op_send = IcePy.Operation('send', Ice.OperationMode.Normal, Ice.OperationMode.Normal, False, (), (((), _M_askap.logging.interfaces._t_LogEvent),), (), None, ())

    _M_askap.logging.interfaces.ILogger = ILogger
    del ILogger

    _M_askap.logging.interfaces.ILoggerPrx = ILoggerPrx
    del ILoggerPrx

# End of module askap.logging.interfaces

__name__ = 'askap.logging'

# End of module askap.logging

__name__ = 'askap'

# End of module askap
