#!/usr/bin/env python
# @file
# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
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
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
import os, re
import sys

from askapdev.rbuild.dependencies.depends import Depends
from askapdev.rbuild.debian import *
from askapdev.rbuild.utils import q_print

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage: debianise.py <package_path or . >"
        sys.exit(1)
    rdir = sys.argv[1]
    a_root = os.getenv('ASKAP_ROOT')
    myself = os.path.relpath(rdir, a_root)
    d = Depends(rdir)
    for dep in d.ordered_dependencies+[myself]:
        nodeb = os.path.join(a_root, dep, "NO_DEBIAN")
        if os.path.exists(nodeb):
            q_print("Ignoring package '{0}' which set to NO_DEBIAN".format(dep))
            continue
        name, version = get_versioned_name(dep)
        add_debian(dep)
    
