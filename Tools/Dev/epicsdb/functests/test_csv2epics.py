#!/usr/bin/python
#
# Copyright (c) 2010 CSIRO
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
'functional test of weather station emulator'
import os
import sys
import filecmp
from time import sleep
from nose.tools import assert_equals, assert_true

class TestEpicsDb(object):
    def test_convert(self):
        if os.access('functests/test.db', os.F_OK):
            os.remove('functests/test.db')
        cmd = 'csv2epics.py --no-comment --prefix="\$(prefix)\$(antid)drx:s1:b0:" --input functests/test.csv --input=functests/test-misc.csv --patch=functests/test-patch.csv --output=functests/test.db'
        os.system(cmd)
        assert_true(filecmp.cmp('functests/verify.db', 'functests/test.db'))
