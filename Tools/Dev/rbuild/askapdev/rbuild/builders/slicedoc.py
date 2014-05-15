# Copyright (c) 2010 CSIRO
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
import os
import glob
from builder import Builder
import askapdev.rbuild.utils as utils

class SliceDoc(Builder):
    def __init__(self, pkgname=None):
        Builder.__init__(self,
                         pkgname=pkgname,
                         archivename=None,
                         buildsubdir=None,
                         buildcommand='slice2html',
                         buildtargets=[])
        '''
        The constructor sets up a package build 'environment'

        :param pkgname:        The (optional) name of the package directory.
                               By default the current directory name is used.
        :param archivename:    The (optional) archive name minus suffix.
                               The default is None.
        :param buildsubdir:    The (optional) directory in which to start the
                               build.
        :param buildtargets:   The (optional) additional build targets.
        :param buildcommand:   The (optional) build command.
                               The default is 'slice2html'.
        '''
        self._docdir = 'doc'
        self.add_extra_clean_targets(self._docdir)
        self.parallel = False
        self.header = None
        self.footer = None
        # get the environment to pick up ice
        pth = os.path.pathsep.join([os.environ.get('PATH'),
                                    self.dep.get_path()])
        self.add_ld_library_paths(self.dep.get_ld_library_path())
        self.append_env('PATH', pth)


    def _add_slice_options(self):
        self.add_option('-I.')
        self.add_option('--output-dir %s' % self._docdir)
        if self.header is not None and os.path.exists(self.header):
            self.add_option('--hdr %s' % self.header)
        if self.footer is not None and os.path.exists(self.footer):
            self.add_option('--ftr %s' % self.footer)


    def _build(self):
        pass


    def _install(self):
        if not os.path.exists(self._installdir):
            os.makedirs(self._installdir)


    def _doc(self):
        self._add_slice_options()
        slicefiles = glob.glob('*.ice')
        slicefiles = ' '.join(slicefiles)
        if utils.in_code_tree():
            utils.run('%s %s %s' % (self._bcom, self._opts, slicefiles),
                      self.nowarnings)
