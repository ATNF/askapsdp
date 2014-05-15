## Package for various utility functions to execute build and shell commands
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
import sys

from ..exceptions import BuildError
from run import run

OPTIONS = sys.argv[1:]
OPTIONSTRING = ' '.join(OPTIONS)

def run_scons(extraargs='', version=None, extraquiet=False):
    '''execute the scons command passing on vaid command-line options
    and massaging them to be scons compliant
    '''
    scons = "scons"
    if version:
        scons = "%s-%s" % (scons, version)
    # ignore pylint
    if "pylint" in OPTIONS:
        return

    #validopts = ["install", "test", "doc", "-q"]
    opts = OPTIONS[:] #[ opt for opt in OPTIONS if opt in validopts ]
    # scons uses -Q not -q to silence it
    if "-q" in opts:
        opts.remove("-q")
        opts.append("-Q")
    if "-u" in opts:
        opts.remove("-u")
        opts.append("update=1")
    if "clean" in opts:
        opts.remove("clean")
    if "-x" in opts:
        opts.remove("-x")
    if "-noparallel" in opts:
        opts.remove('-noparallel')

    optstr = ' '.join(opts) + " " + extraargs
    # don't redirect stdout
    # XXX - Tony - I do not understand why test was treated differently.
    #     Now we do want it treated differently as we now expect the unit tests
    #     to give a correct exit status on a failure and do not want to
    #     raise a BuildError.
    #     But why was it previously treated differently?
    if "test" in opts:
        sconscom = "%s %s" % (scons, optstr)
        os.system(sconscom)
        #if os.system(sconscom) > 0:
        #    raise BuildError(sconscom)
    else:
        # also build tests here if not in 3rdParty,
        # so they don't get built on execute.
        if "install" in opts and \
                os.path.abspath(os.curdir).find("3rdParty") == -1:
            optstr += " buildtests"
        run("%s %s" % (scons, optstr), extraquiet=extraquiet)
