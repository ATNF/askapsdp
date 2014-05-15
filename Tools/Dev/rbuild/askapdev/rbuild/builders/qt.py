## @qt.py
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
# @author Juan Carlos Guzman <Juan.Guzman@csiro.au>
#
import os

from builder import Builder
import askapdev.rbuild.utils as utils

## Implementation of Builder for Qt4 projects (qmake and make).
#  It overwrites _configure, _build and _install to reflect the packages
#  build tools
class Qt(Builder):
    ## The constructor sets up a package build "environment"
    #  @param self           The current object
    #  @param pkgname the name of the package directory. By default the
    #  current directories (tag) name is used
    #  @param archivename [optional] alternate tarball name
    #  if it minus suffix (e.g. .tar.gz) differs from the pkgname
    #  @param buildsubdir [optional] directory in which to start the build.
    #  @param buildtargets [optional] additional build targets.
    #  @param qtdep the key name of Qt in the dependencies file. Default is 'qt4'
    def __init__(self, pkgname=None, archivename=None, buildsubdir=None,
                 buildtargets=[], buildcommand='make', confcommand=None,
                 installcommand='make install', qtdep='qt4'):
        Builder.__init__(self, pkgname=pkgname,
                                   archivename=archivename,
                                   buildsubdir=buildsubdir,
                                   buildcommand=buildcommand,
                                   buildtargets=[""]+buildtargets,
                                   confcommand=confcommand,
                                   installcommand=installcommand)
        # Set path to Qt binaries (where qmake is located)
        self.qtpath = self.dep.get_install_path(qtdep)
        self._ccom = os.path.join(self.qtpath, 'bin', 'qmake')
        # Add auto-generated Makefile to clean targets (prevent adding Makefile to repository)
        self.add_extra_clean_targets('Makefile')
        
    def _configure(self):
        if self._ccom:
            utils.run("%s %s \"PKG_INSTALL_BASE=%s\"" % (self._ccom, self._opts, self._prefix), self.nowarnings)
                
    def _build(self):
        if self._bcom:
            for btgt in self._btargets:
                utils.run("%s %s %s" % (self._bcom, self.get_parallel_opt(), btgt),
                          self.nowarnings)

    def _clean(self):
        if os.path.exists(self._package):
            curdir = os.path.abspath(os.curdir)
            # Enter the untarred package directory
            os.chdir(self._package)
            if os.path.exists('Makefile'):
                utils.run("make clean")
            os.chdir(curdir)
        # Delete the reminder of the directories
        Builder._clean(self)
