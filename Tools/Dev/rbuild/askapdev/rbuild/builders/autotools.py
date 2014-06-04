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

from builder import Builder
import askapdev.rbuild.utils as utils

platform = utils.get_platform()
if platform['system'] == 'FreeBSD':
    MAKE = 'gmake'
else:
    MAKE = 'make'
CONFIG  = 'configure'
INSTALL = MAKE + ' install'

class Autotools(Builder):
    '''
    A subclass of Builder for autotools (configure/make/make install).
    It overwrites _configure(), and _build()  and initializes buildcommand and
    installcommand.
    '''
    def __init__(self, pkgname=None, archivename=None, buildsubdir=None,
                 buildtargets=[], confcommand=CONFIG, buildcommand=MAKE,
                 installcommand=INSTALL):
        '''
        The constructor sets up a package build "environment"

        :param pkgname:        The name of the package directory.
                               By default the current directory name is used
        :param archivename:    The (optional) archive name minus suffix.
                               The default is based on package name.
        :param buildsubdir:    The (optional) directory in which to start the
                               build.
        :param buildtargets:   The (optional) additional build targets.
        :param confcommand:    The (optional) configure command.
                               The default is 'configure'.
        :param buildcommand:   The (optional) build command.
                               The default is 'make'.
        :param installcommand: The (optional) install command.
                               The default is 'make install'.
        '''
        Builder.__init__(self, pkgname=pkgname,
                               archivename=archivename,
                               buildsubdir=buildsubdir,
                               buildtargets=[""]+buildtargets,
                               buildcommand=buildcommand,
                               confcommand=confcommand,
                               installcommand=installcommand)

    def _configure(self):
        if self._ccom:
            if os.path.abspath(os.curdir).find("Tools") > 0:
                self._prefix = self._askaproot
            cmd = "./%s %s --prefix=%s" % (self._ccom, self._opts, self._prefix)
            utils.run(cmd, self.nowarnings)


    def _build(self):
        if self._bcom:
            for btgt in self._btargets:
                cmd = "%s %s %s" % (self._bcom, self.get_parallel_opt(), btgt)
                utils.run(cmd, self.nowarnings)
