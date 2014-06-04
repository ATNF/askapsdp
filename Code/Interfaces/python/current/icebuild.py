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

from __future__ import with_statement
import os
import glob
from setuptools import Command

from askapdev.rbuild.setup.commands import clean as rbuild_clean

class build_ice(Command):
    description = "Generate ice stubs using slice2py"
    user_options = []
    # @param self the current object
    def initialize_options (self):
        self.interfaces = []
        self.package = ""
        self.dep = None

    def finalize_options (self):
        pass

    def run (self):
        incdirs = []
        for slice in self.interfaces:
            dir = os.path.split(slice)[0]
            if dir:
                incdirs.append("-I"+dir)
        pkgdir = self.package.replace(".", os.path.sep)
        if not os.path.exists(pkgdir):
            raise IOError("Target package directory '%s' not found" % pkgdir)
        slicepy = os.path.join(self.dep.get_install_path("ice"),
                               "bin", "slice2py")
        os.system("%s %s --no-package --output-dir %s %s" \
                      % (slicepy, " ".join(incdirs), pkgdir, 
                         " ".join(self.interfaces)))
        sliced = glob.glob(os.path.join(pkgdir, "*_ice.py"))
        for slice in sliced:
            out = slice.replace('_ice', '')
            name = os.path.splitext(os.path.basename(slice))[0]
            with open(out, 'w') as f:
                f.write('# Auto-generated. Do NOT modify.\n')
                f.write('import Ice\n')
                f.write('from . import %s\n' % name)
                f.write('Ice.updateModules()\n')

class clean_ice(rbuild_clean):
    def run(self):
        ice = []
        for base,dirs,files in os.walk(os.curdir):
            for f in files:
                if f.find("_ice.py") >= 0:
                    ice += [ os.path.join(base, f) for d in dirs ]
        generated = [ f.replace('_ice', '') for f in ice]
        for p in ice+generated:
            if os.path.exists(p):
                if os.path.isdir(p):
                    remove_tree(p)
                else:
                    os.remove(p)
        rbuild_clean.run(self)
