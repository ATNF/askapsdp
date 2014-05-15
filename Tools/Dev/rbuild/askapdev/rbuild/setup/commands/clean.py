## Module defining the clean target command. It overloads distutils.clean
# command but forces remove all and also deletes dist directory.
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
# @author Robert Crida <robert.crida@ska.ac.za>
#

import distutils.command.clean
from distutils.dir_util import remove_tree
import os
import os.path
import glob
import shutil

## This is an extension command to be used by setup to provide a "clean" target.
class clean(distutils.command.clean.clean):
    """Command to clean project"""
    ## The description string of this Command class
    description = "clean the project"

    ## Execute the target by running doxygen according to the options.
    # @param self
    def run (self):
        ## flag to indicate whether to remove all
        self.all = True # force remove all
        distutils.command.clean.clean.run(self)
        #tests = ["tests-results.xml", "functests-results.xml"]
        docs = ["doc/_build"]
        vfile = "version.py"
        extras = []
        for base,dirs,files in os.walk(os.curdir):
            for f in files:
                if f == vfile:
                    extras += [ os.path.join(base, f) for d in dirs ]

        einf = glob.glob("*.egg-info")
        cleandirs = ["dist", "temp"] + einf + extras + docs
        for p in cleandirs:
            if os.path.exists(p):
                if os.path.isdir(p):
                    remove_tree(p)
                else:
                    os.remove(p)
