## Module defining the pylint target command. This provides a facade to pylint.
#
# @copyright (c) 2006 CSIRO
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
# @date 2006-12-14
#

from setuptools import Command
from distutils.errors import DistutilsOptionError
import os

## This is an extension command to be used by setup to provide a "pylint" target.
class pylint(Command):
    """Command to inspect code using pylint"""

    ## The description string of this Command class
    description = "inspect code using pylint"

    ## List of option tuples: long name, short name (None if no short
    # name), and help string.
    user_options = [('disable=', 'd', "Disable the pylint build"),
                    ('package=', 'p', "Specify the package root")
                   ]

    ## Initialise all the option member variables
    # @param self the current object
    def initialize_options (self):
        ## option to disable running pylint
        self.disable = None
        ## option setting the pylint configuration file
        self.rcfile = None
        ## option setting the package root directory
        self.package = None

    ## Finalize the option member variables after they have been configured
    # by command line arguments or the configuration file. Needs to ensure that
    # they are valid for processing.
    # @param self the current object
    def finalize_options (self):
        if self.disable == None:
            self.disable = False
        if self.disable == True:
            return
        if self.rcfile is None:
            askapRootDir = os.environ["ASKAP_ROOT"]
            self.rcfile = "%s/pylintrc" % askapRootDir
        if self.package is None:
            raise DistutilsOptionError("You must specify a module package")

    ## Execute the target by running pylint according to the options.
    # @param self
    def run (self):
        if self.disable == False:
            import os
            askapRootDir = os.environ["ASKAP_ROOT"]
            pylint = "%s/bin/pylint" % askapRootDir
            # Here are the motivations for disabling the messages:
            #   F0401 - importing from eggs and pylint isn't managing
            #   C0111 - using doxygen comment instead of module pydoc
            #   E1101 - can't import modules so don't know what methods are available
            command = pylint + \
                " --rcfile=%s --disable-msg=W0403,W0223,I0011,F0401,C0111,E1101,R0922,R0903 %s" \
                % (self.rcfile, self.package)
            os.system(command)
        else:
            print "warning: pylint disabled (disable = True)"
