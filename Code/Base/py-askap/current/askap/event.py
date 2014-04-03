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
"""
=========================
Module :mod:`askap.event` 
=========================

Additions to the :mod:`threading` module"""
__all__ = ["AbortError", "AbortEvent", "raise_abort"]
from threading import Condition, Lock

class AbortEvent(object):
    """Event class with same interface as :func:`threading.Event` but it
    adds a message which can be use to raise an excption with."""
    def __init__(self):
        self.__message = ""
        self.__cond = Condition(Lock())
        self.__flag = False

    def _reset_internal_locks(self):
        # private!  called by Thread._reset_internal_locks by _after_fork()
        self.__cond.__init__()

    def is_set(self):
        return self.__flag

    def is_raised(self):
        return self.__message and self.__flag 
    
    def message(self):
        return self.__message

    def set(self):
        self.__cond.acquire()
        try:
            self.__flag = True
            self.__message = ""
            self.__cond.notify_all()
        finally:
            self.__cond.release()

    def raise_exception(self, reason):
        """set the event and provide a reason"""
        self.__cond.acquire()
        try:
            self.__flag = True
            self.__message = reason
            self.__cond.notify_all()
        finally:
            self.__cond.release()

    def clear(self):
        self.__cond.acquire()
        try:
            self.__flag = False
            self.__message = ""
        finally:
            self.__cond.release()

    def wait(self, timeout=None):
        self.__cond.acquire()
        try:
            if not self.__flag:
                self.__cond.wait(timeout)
            return self.__flag
        finally:
            self.__cond.release()


class AbortError(Exception):
    """Exception to raise when and :class:`AbortEvent` is set."""
    pass


def raise_abort(event):
    """Raise an :class:`AbortError` when :class:`AbortEvent` is set.

    :param event: the :class:`AbortEvent` to check

    """
    if event.is_raised():
        raise AbortError(event.message())
