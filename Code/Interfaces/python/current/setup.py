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

import glob

from askapdev.rbuild.setup import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages
from setuptools.command.build_py import build_py
from icebuild import build_ice, clean_ice

class build_py2(build_py):
    def run(self):
        for cmd_name in self.get_sub_commands():
            self.run_command(cmd_name)
        build_py.run(self)
     
build_py2.sub_commands.append(('build_ice', None))

dep = Dependency()
dep.add_package()

slice_files = glob.glob('../../slice/current/*.ice')

ROOTPKG = "askap"
PKGNAME = "slice"

setup(name = "%s.%s" % (ROOTPKG, PKGNAME),
      version = 'current',
      description = 'ASKAP package for slice2py generated code.',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'Interfaces', 'Ice'],
      long_description = '',
      packages = find_packages(),
      namespace_packages = [ROOTPKG],
      license = 'GPL',
      zip_safe = 0,
      dependency = dep,
      cmdclass = {'build_ice': build_ice, 
                  'build_py': build_py2,
                  'clean': clean_ice},
      options = { 'build_ice' : { 'interfaces' : slice_files,
                                  'dep' : dep ,
                                  'package' : 'askap.slice' }},
      #test_suite = "nose.collector",
)
