# Copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

__author__ = "robert.crida@ska.ac.za"

from askapdev.rbuild.setup import setup

setup (
    name         = "askapdev.rbuild",
    description  = "scripts for recursively building packages",
    author       = "Tony Maher",
    author_email = "Tony.maher@csiro.au",
    packages     = [ "askapdev",
                     "askapdev.rbuild",
                     "askapdev.rbuild.builders",
                     "askapdev.rbuild.dependencies",
                     "askapdev.rbuild.utils",
                     "askapdev.rbuild.setup",
                     "askapdev.rbuild.setup.commands",
                     "askapdev.rbuild.debian",
                     "templates",
                   ],
    package_data = { "templates": ["*"], 
                     "askapdev.rbuild.debian": 
                     ["data/[!.]*"] },
    scripts      = [ "scripts/rbuild",
                     "scripts/debianise.py",
                     "scripts/askap-debpackage",
                   ],
    zip_safe     = False,
    #
    namespace_packages = ['askapdev'],
)
