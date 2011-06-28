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
from askapdev.rbuild.setup import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

PKGNAME = ROOTPKG = "askap"

setup(name = PKGNAME,
      version = 'current',
      description = 'ASKAP basic modules',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'logging', 'Base'],
      long_description = '''General basic ASKAP modules such as logging
''',
      packages = find_packages(),
      license = 'GPL',
      zip_safe = 0,
      dependency = dep,
      test_suite = "nose.collector",
)
