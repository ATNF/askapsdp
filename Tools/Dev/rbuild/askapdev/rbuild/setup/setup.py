# askapdev.rbuild module containing tools to assist with the build process.
#
# @copyright (c) 2006-2011 CSIRO
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
# @author Robert Crida <robert.crida@ska.ac.za>
#

import sys
import os
import re
import setuptools

from  . import commands
from .. import utils

## Recursivebuild incarnation of the distutils.core.setup() function
# This version simply registers ASKAPsoft specific additional commands
# to support new build targets.
# @param attrs the dictionary of attributes
def setup(**attrs):
    '''
    :param attrs: dictionary

    Example::
    
        # strip the -x and -u flags from the command line if necessary
        if sys.argv.count('-x'):
            sys.argv.remove('-x')
        if sys.argv.count('-u'):
            sys.argv.remove('-u')

        # check for the version by reading the directory name
        version = os.getcwd().split(os.sep)[-1]
        if "version" in attrs and attrs["version"] != version:
            msg = "Version discrepancy between setup.py and directory name"
            raise RuntimeError(msg)

    '''
    ## This auto-generates a version.py file in the package directory
    #  It needs to be removed by 'clean' (see commands/clean.py)
    if "Code" in os.getcwd():
        versionstr = utils.get_release_version()
        pkgpath = os.path.join(*(attrs["name"].split(".")))
        # ignore non-existing packages such as in functests
        if os.path.exists(pkgpath):
            vfilename = os.path.join(pkgpath, "version.py")
            with open(vfilename, 'w') as vfile:
                vfile.write("""# Auto-generated - DO NOT EDIT OR CHECK IN
ASKAP_VERSION = "%s"
""" % ( versionstr,))
    if "dependency" in attrs:
        dep = attrs["dependency"]
        # check if we are compiling extensions, in which case automatically
        # add details for dependencies
        if "ext_modules" in attrs:
            for ext in attrs["ext_modules"]:
                ext.include_dirs += dep.get_includedirs()
                ext.library_dirs += dep.get_librarydirs()
                ext.libraries += dep.get_libs()
                ext.runtime_library_dirs += dep.get_librarydirs()

        # now remove from dict so that rest of setup doesn't complain!
        del attrs["dependency"]

        if not "options" in attrs:
            attrs["options"] = {}
        attrs["options"]["doc"] = { 'dep' : dep }

    newcmds = { "clean": commands.clean,
                "doc": commands.doc,
                "pylint": commands.pylint,
                }

    if "cmdclass" in attrs:
        # don't overwrite custom ones
        for k, v in newcmds.items():
            if k not in attrs["cmdclass"]:
                print "Adding target", k
                attrs["cmdclass"][k] = v
    else:
        attrs["cmdclass"] = newcmds

    setuptools.setup(**attrs)
