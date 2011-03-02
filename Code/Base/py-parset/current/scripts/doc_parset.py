#!/usr/bin/env python
#
# Copyright (c) 2011 CSIRO
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

import sys
import os

from askap.parset import ParameterSet, encode

if len(sys.argv) != 2:
    print """usage:
\t %s <parmeterset_file>
""" % (os.path.basename(sys.argv[0]) )
    sys.exit(1)

p = ParameterSet(sys.argv[1])
buffer = ""
title = "Documentation for "+ os.path.basename(sys.argv[1])+"\n"
title += "="*(len(title)-1)+"\n"

print title
for k in p.keys():
    print "**"+k+"**", "   = :literal:`%s`" % encode(p[k])
    doc = p.get_doc(k)
    for line in doc.split("\n"):
        print " "*2, line
    print

