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


## Implementation of Builder for scons.
#  It overwrites _configure, _build and _install to reflect the packages
#  build tools
class Scons(Builder):
    ## The constructor sets up a package build "environment"
    #  @param self           The current object
    #  @param pkgname the name of the package directory. By default the
    #  current directories (tag) name is used
    #  @param archivename an optional alternate tarball name
    #  if it minus suffix (e.g. .tar.gz) differs from the pkgname
    #  @param extractdir if the tarball extracts to the current directory
    #  rather than a subdirectory, set this so a subdirectory is created
    #  and the tarball extracts into this subdirectory.
    #  @param sconsversion (optional) version of scons to use. If not
    #  specified, to use the default 'scons' defined by the build system.
    def __init__(self, pkgname=None, archivename=None, extractdir=None,
                 sconsversion=None):
        Builder.__init__(self, pkgname=pkgname,
                         archivename=archivename,
                         extractdir=extractdir)
        self.sconsversion = sconsversion

    def _build(self):
        pass

    def _testtgt(self, tgt):
        utils.run_scons(extraargs=tgt, version=self.sconsversion)

    def _test(self):
        if os.path.exists("tests"):
            self._testtgt("tests")
        else:
            print "error: Cannot run tests as the tests subdirectory is "\
                  "missing: %s" % os.path.join(self._bdir, "tests")

    def _functest(self):
        if os.path.exists("functests"):
            self._testtgt("functest")
            Builder._functest(self)
        else:
            print "error: missing functests subdirectory in %s" % \
                    os.path.relpath(self._bdir, self._askaproot)


    def _doc(self):
        utils.run_scons(version=self.sconsversion)

    def _clean(self):
        try:
            utils.run_scons(extraargs="--clean . tidy", version=self.sconsversion)
        except Exception, inst:
            utils.q_print("info: could not run %s. Will try our clean." % inst)
        return Builder._clean(self)

    def _install(self):
        # ignore pylint target
        if "pylint" in self._comopts:
            return
        self._opts += self.get_parallel_opt()
        btgt = "%s %s prefix=%s" % (' '.join(self._btargets),
                                    self._opts, self._prefix)
        utils.run_scons(extraargs=btgt, version=self.sconsversion,
                        extraquiet=self.nowarnings)
        self._version_install()
