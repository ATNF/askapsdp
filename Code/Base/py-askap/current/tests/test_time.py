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
import pytz
from nose.tools import assert_equals, raises
from askap.time import (bat2utc, utc2bat,
                        siteID2LocTimezone, SiteError)


def test_bat2utc():
    raise RuntimeError("to be implemented")

def test_utc2bat():
    raise RuntimeError("to be implemented")

@raises(SiteError)
def test_site_raises():
    val = siteID2LocTimezone('blah')


# run tests for all dictionary items
def site(key, value):
    tz = siteID2LocTimezone(key)
    assert_equals(tz.zone, value)

def test_sites():
    mapping = { 'MRO': 'Australia/Perth',
                'MATES': 'Australia/Sydney',
                'PKS': 'Australia/Sydney',
                }
    for k,v in mapping.items():
        yield site, k,v

    
