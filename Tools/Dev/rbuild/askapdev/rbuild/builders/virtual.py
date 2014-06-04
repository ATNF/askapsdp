# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
# @copyright (c) 2007 CSIRO
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
import os
import re
import subprocess
import sys

from builder import Builder
import askapdev.rbuild.utils as utils

## Implementation of Builder for 'modules' which will be treated as a
#  virtual package
#  It overwrites _configure, _build and _install to reflect the packages
#  build tools
class Virtual(Builder):
    ## The constructor sets up a package build "environment"
    #  @param self           The current object
    #  @param pkgname the name of the modules package e.g. openmpi
    #  @param exename the name of an executable  e.g. mpicxx

    def __init__(self, pkgname, pkgversion=None, exename=None):
        Builder.__init__(self, pkgname=pkgname)
        # Needed for os.chdir in _build
        self._pkgversion = pkgversion
        self._exename = exename
        self._builddir = "."
        self.parallel = False

    def _get_module_root(self):
        '''Try to find the virtual package using environment modules'''
        # The 'module' command is not a binary but a shell function so need
        # to use its initialisation procedure.
        try:
            modpth = os.path.join(os.environ["MODULESHOME"], "init/sh")
        except KeyError:
            return None

        pkg = self._package
        if self._pkgversion is not None:
            pkg = os.path.join(pkg, self._pkgversion)
        cmd = ". %s; module display %s" % (modpth, pkg)
        proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
        output = proc.communicate()[0]
        if proc.returncode > 0:
            print  "error: 'modules display' failed.  Error message: '%s'" \
                    % output
            sys.exit(1)
        # Take the root from the PATH entry
        rx = re.compile(".*PATH\s+(.+)/bin.*")
        try:
            root = rx.search(output).groups()[0]
        except:
            root = False
        return root

    def _get_path_root(self):
        '''Try to find the virtual package using PATH environment variable.'''
        if self._exename:
            return utils.which(self._exename).rpartition('/bin/')[0]
        else:
            return None

    def _install(self):
        pkgroot = self._get_module_root() or self._get_path_root()
        if pkgroot:
            if os.path.islink('install'):
                os.remove('install')
            os.symlink(pkgroot, 'install')
        else:
            print "warn: Unable to find a system package so cannot create virtual package %s." % self._package


    def _stage(self):
        return
