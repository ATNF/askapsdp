# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
# @copyright (c)92007 CSIRO
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

from builder import Builder
import askapdev.rbuild.utils as utils
from ..exceptions import BuildError


class Data(Builder):
    '''
    Implementation of Builder for just data files/directories.
    It overwrites _configure, _build and _install to reflect the packages
    build tools
    '''
    def __init__(self, pkgname=None, archivename=None, extractdir=None):
        '''
        The constructor sets up a package build "environment"
        :param pkgname:        The (optional) name of the package directory.
                               By default the current directory name is used.
        :param archivename:    The (optional) archive name minus suffix.
                               The default is based on package name.
        :param extractdir:     The (optional) directory into which the archive
                               is extracted. It is created if it does not exist.
        '''
        if pkgname == '.':
            raise BuildError("The package name has to be explicit")
        Builder.__init__(self, pkgname=pkgname,
                         archivename=archivename,
                         extractdir=extractdir)
        if self._tarname is not None:
            self.add_extra_clean_targets(self._package)
        self.no_initscript = True
        self.parallel = False


    def _build(self):
        pass


    def _install(self):
        os.chdir('..')
        utils.copy_tree(self._package, self._installdir, overwrite=True)
