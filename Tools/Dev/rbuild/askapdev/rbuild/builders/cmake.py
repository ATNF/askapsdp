## @file
# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
# @copyright (c) 2007-2011 CSIRO
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

platform = utils.get_platform()
if platform['system'] == 'FreeBSD':
    MAKE = 'gmake'
else:
    MAKE = 'make'

class CMake(Builder):
    '''
    Implementation of Builder for autotools (make and configure).
    It overwrites _configure, and _build to reflect the packages
    build tools.
    '''
    def __init__(self, pkgname=None, archivename=None, buildsubdir=None,
                 buildtargets=[], buildcommand=MAKE, confcommand="cmake",
                 installcommand=" ".join([MAKE, "install"])):
        '''
        The constructor sets up a package build "environment"

        :param pkgname:        The (optional) name of the package directory.
                               By default the current directory name is used.
        :param archivename:    The (optional) archive name minus suffix.
                               The default is based on package name.
        :param extractdir:     The (optional) directory into which the archive
                               is extracted. It is created if it does not exist.
        :param buildsubdir:    The (optional) directory in which to start the
                               build.
        :param buildtargets:   The (optional) additional build targets.
        :param confcommand:    The (optional) command to configure the package.
                               The default is 'cmake'.
        :param buildcommand:   The (optional) build command.
                               The default is 'make'.
        :param installcommand: The (optional) install command.
                               The default is 'make install'.
        '''
        Builder.__init__(self,
                         pkgname=pkgname,
                         archivename=archivename,
                         buildsubdir=buildsubdir,
                         buildcommand=buildcommand,
                         buildtargets=['']+buildtargets,
                         confcommand=confcommand,
                         installcommand=installcommand)

        self._cmakedir = 'build'


    def _configure(self):
        if self._ccom:
            if os.path.abspath(os.curdir).find('Tools') > 0:
                self._prefix = self._askaproot
            if not os.path.exists(self._cmakedir):
                os.makedirs(self._cmakedir)
            os.chdir(self._cmakedir)
            utils.run('%s %s -DCMAKE_INSTALL_PREFIX=%s ..' %
                      (self._ccom, self._opts, self._prefix), self.nowarnings)


    def _build(self):
        if self._bcom:
            for btgt in self._btargets:
                utils.run('%s %s %s' % (self._bcom, 
                                        self.get_parallel_opt(), btgt),
                                        self.nowarnings)
