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
from askap.logging import Handler, log_debug, getLogger, DEBUG


class ListHandler(Handler):
    def __init__(self):
        Handler.__init__(self)
        self.event = None

    def emit(self, record):
        self.event = [record.name, record.getMessage()]

# This is passes in as reference as it is a list. Use this to access it globally
#event = None
hand = ListHandler()
logger = getLogger(__name__)
logger.setLevel(DEBUG)
logger.addHandler(hand)

# pylint: disable-msg=W0613
@log_debug
def debug_me(arg, kwarg=2):
    pass
    
def test_log_debug():
    debugmestr = 'debug_me:  (1, 2) '
    debugmename = "tests.test_logging"
    # call function to generate log message
    debug_me(1, 2)
    # check the last generated log event
    assert debugmename == hand.event[0]
    assert debugmestr == hand.event[1]

