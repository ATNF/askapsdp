## Module for defining the doc command. This provides a facade to sphinx.
#
# @copyright (c) 2009 CSIRO
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
# @author Malte Marquarding <malte.marquarding@csiro.au>
# @date 2009-06-18
#
from __future__ import with_statement
"""This is an extension command to be used by setup to provide a "doc" target.
It is intended that it have an option - format - to allow
specification of the output format of sphinx.
"""
from setuptools import Command
import pickle
import os

class doc(Command):
    """Command to build documentation using sphinx"""
    description = "build documentation using sphinx"

    ## List of option tuples: long name, short name (None if no short
    # name), and help string.
    user_options = [('format=', 'f',
                     "The documentation output format e.g. html (default)"),
                   ]
    ##
    # @param self the current object
    def initialize_options (self):
        """
        Initialise all the option member variables
        """
        self.format = None
        self.dep = None

    def finalize_options (self):
        """
        Finalize the option member variables after they have been configured
        by command line arguments or the configuration file. Needs to ensure
        that they are valid for processing.
        """
        if self.format is None:
            self.format = 'html'

    def run (self):
        """Execute the target by running sphinx according to the options."""
        import os
        import subprocess
        pth = "doc"
        tgt = "Makefile"
        if not os.path.exists(os.path.join(pth, tgt)):
            print "warn: Documentation not found ('doc/Makefile')"
            return
        os.chdir(pth)
        crossrefs = {}
        reffile = "crossrefs.py"
        if self.dep:
            dirs = self.dep.get_rootdirs()
            for d in dirs:
                p = os.path.join(d, "doc", "_build", "html")
                if os.path.exists(os.path.join(p, "objects.inv")):
                    name = d.split(os.path.sep)[-2].replace("-", "")
                    crossrefs[name] = (p, None)
        with open(reffile, 'w') as f:
            f.write("intersphinx_mapping=%s\n" % str(crossrefs))
        try:
            os.system("make %s" % self.format)
        finally:
            if os.path.exists(reffile):
                os.remove(reffile)
            if os.path.exists(reffile+"c"):
                os.remove(reffile+"c")
