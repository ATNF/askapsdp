## @file
# @copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# @author Tony Maher <Tony.Maher@csiro.au>
#

# In scons, tools must live in a directory and be specified by a file system
# path.  They cannot be in a python package.
# We want to install into the top level ASKAPsoft tree (like all other tools)
# and not hard coded to a location in the Tools subdirectory.

import glob
import os
import shutil

from askapdev.rbuild.builders import Builder

class myBuilder(Builder):
    def __init__(self):
        Builder.__init__(self, pkgname='.', archivename=None, extractdir=None)

    def _build(self):
        pass

    def _install(self):
        files = glob.glob("*.py")
        ASKAP_ROOT = os.environ['ASKAP_ROOT']
        SCONS_TOOLS_DIR = os.path.join(ASKAP_ROOT, 'share', 'scons_tools')

        if not os.path.exists(SCONS_TOOLS_DIR):
            os.makedirs(SCONS_TOOLS_DIR)

        for file in glob.glob('*.py'):
            if file != 'build.py':
                shutil.copy(file, SCONS_TOOLS_DIR)


builder = myBuilder()
builder.build()
