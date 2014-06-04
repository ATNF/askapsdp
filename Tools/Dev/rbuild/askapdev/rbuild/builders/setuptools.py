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

import glob
import sys
import os

from builder import Builder
import askapdev.rbuild.utils as utils


## Implementation of Builder for easy_install.
#  It overwrites _configure, _build and _install to reflect the packages
#  build tools
class Setuptools(Builder):
    ## The constructor sets up a package build "environment"
    #  @param self           The current object
    #  @param pkgname the name of the package directory. By default the
    #  current directories (tag) name is used
    #  @param archivename an optional alternate tarball name
    #  if it minus suffix (e.g. .tar.gz) differs from the pkgname
    #  @param buildsubdir [optional] the sub-directory in which to do the build
    def __init__(self, pkgname=None, archivename=None, buildsubdir=None):
        Builder.__init__(self, pkgname=pkgname, buildsubdir=buildsubdir,
                         archivename=archivename)
        self.parallel = False
        self.has_extension = False
        self._pycmd = os.path.abspath(os.path.join(self._askaproot, 'bin', 'python'))

    ## If building in Tools subdirectory, we want to install them in the
    #  ASKAPsoft root tree so that we can use them during build.
    #  If building in 3rdParty we want to install them in the build
    #  directory so they can be packaged up for deployment.
    #

    def _build(self):
        if not self.has_extension:
            return
        cmd = "%s setup.py build_ext %s" % (self._pycmd, self._opts)
        utils.run("%s" % cmd, self.nowarnings)

    def _install(self):
        if os.path.exists("setupegg.py"):
            cmd = "%s setupegg.py install" % self._pycmd
        else:
            cmd = "%s setup.py install" % self._pycmd

        if os.getcwd().find("Tools") > 0:
            msg = "info: 'setuptools egg install', imports use default version."
        else:
            msg = "info: 'setuptools in local 'install' directory."
            installdir = os.path.join(self._bdir, self._installdir)
            cmd += " --prefix %s" % installdir
            utils.create_python_tree(installdir)

        utils.run("%s %s" % (self._get_env(), cmd), self.nowarnings)
        utils.q_print(msg)
        self._version_install()

    def _doc(self):
        if not utils.in_code_tree() and not utils.in_dev_tree():
            return
        env = self._get_env()
        cmd = "%s %s setup.py doc" % (env, self._pycmd)
        utils.run("%s" % cmd, self.nowarnings)

    def _clean(self):
        # Cleaning in Tools is done explicitly in rbuild script
        # and do not want it run a second time and complaining.
        if not 'Tools' in os.getcwd(): # i.e. in 3rdParty and Code
            if os.path.exists("setup.py"):
                utils.run("%s setup.py clean" % self._pycmd)
        Builder._clean(self)

    def _testtgt(self, target):
        env = self._get_env()
        # handle explicit test target
        filetarget = target
        for o in self._comopts:
            if o.startswith(target+"/"):
                filetarget = o
        nose = self._pycmd.replace("python", "nosetests")
        cmd = "%s %s --with-xunit --xunit-file=%s/%s-junit-results.xml"\
              " %s" % (env, nose, target, target, filetarget)
        utils.run("%s" % cmd, self.nowarnings, ignore_traceback=True)

    def _test(self):
        if not utils.in_code_tree():
            return
        if os.path.exists("tests"):
            self._testtgt("tests")
        else:
            print "error: Cannot run tests as the tests subdirectory is missing: %s" % os.path.join(self._bdir, "tests")

    def _functest(self):
        if not utils.in_code_tree():
            return
        if os.path.exists("functests"):
            self._testtgt("functests")
        else:
            print "error: missing functests subdirectory in %s" % \
                    os.path.relpath(self._bdir, self._askaproot)
