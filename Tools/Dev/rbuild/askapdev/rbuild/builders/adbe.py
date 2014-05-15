## @file
# builder to parse XML point definitions in adbe common library
# and generate epics database, asyn glue code and emulator update
# utilities.
#
# @copyright (c) 2007-2013 CSIRO
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
# @author Craig Haskins <Craig.Haskins@csiro.au>
#

import os
from builder import Builder
import askapdev.rbuild.utils as utils
from askapdev.epicsdb import AdbeParser
from ..exceptions import BuildError

class Adbe(Builder):
    def __init__(self, pkgname=None, archivename=None, extractdir=None,
                 epicsbase=None, releasefile=None, srcdir=None,
                 fileprefix=None, dbprefix=None, *args, **kwargs):
        Builder.__init__(self, pkgname=pkgname,
                               archivename=archivename,
                               extractdir=extractdir,
                               buildcommand="",
                               buildtargets=[""])
        self.parallel = False
        self._appname = os.getcwd().split(os.sep)[-2]

        if epicsbase is None:
            self._epicsBuild = False
            self._srcdir = srcdir
            srcOutDir = srcdir
            dbOutDir = None
            xmlOutDir = None
        else:
            self._epicsBuild = True
            self._epicsbase = self.dep.get_install_path(epicsbase)
            self._epicsbase = self._epicsbase.rstrip(os.sep)
            self._srcdir = os.path.join(srcdir, 'src')
            comm = os.path.join(os.path.dirname(self._epicsbase), "EpicsHostArch")
            #self._epicsarch = utils.runcmd(comm)[0].strip() + '-debug'
            self._epicsarch = utils.runcmd(comm)[0].strip()

            srcOutDir = os.path.join(srcdir, 'src', 'O.' + self._epicsarch)
            dbOutDir = os.path.join(srcdir, 'Db', 'O.Common')
            xmlOutDir = os.path.join('install', 'epicsxml')

        self._adePoints = []
        self._adeParser = AdbeParser(self._appname, dbprefix, fileprefix, srcOutDir, dbOutDir, xmlOutDir, *args, **kwargs)
            
    def add_points(self, header, library=None, epicsxml=False):
        '''
        add the specified header file to the list of modules
        to parse for XML point definitions

        :param library: an optional library name, if header belongs to a library
		:param epicsxml: True to output XML files for GUI/OSL
        '''
        self._adePoints.append( (header, library, epicsxml) )

    def _build(self):
        '''
        Parse the header files listed file add_points.  The parsing involves

        1. extracting XML tags from the header file
        2. parsing the XML to a document tree
        3. expanding out any nodes such as iocStructures or iocArrays
        '''
        sourceFiles = []
        for header, library, epicsxml in self._adePoints:
            if library is None:
                filename = os.path.join(self._srcdir, header + '.h')
            else:
                filename = os.path.join(self.dep.get_install_path(library), 'include', library, header + '.h')

            sourceFiles.append((filename, library, epicsxml))

        if self._adeParser.rebuild_needed([x[0] for x in sourceFiles]):
            self._adeParser.reset()
            for filename, library, epicsxml in sourceFiles:
                self._adeParser.parse(filename, libName=library, epicsxml=epicsxml)

            files = self._adeParser.generate_output()
            for file in files:
                self.add_extra_clean_targets(file)
                utils.q_print("generated " + file)
            
    def _install(self):
        Builder._install(self)
        self._adeParser.install()

    def _clean(self):
        Builder._clean(self)
