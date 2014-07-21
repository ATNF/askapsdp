# @file get_vcs_type.py
# Determine the version control system
#
# @copyright (c) 2014 CSIRO
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
# @author Ben Humphreys <ben.humphreys@csiro.au>
#

import os
from runcmd import runcmd

def is_git():
    return is_vcs_check('.git', 'git status -s')

def is_svn():
    return is_vcs_check('.svn', 'svn info')

# Helper function. Checks if "testdir" is present in $ASKAP_ROOT, and if not
# will probe the VCS by exectuing a command. If the "testdir" exists or the
# "probecmd" returns a code of 0, true is returned, otherwise false.
def is_vcs_check(testdir, probecmd):
    try:
        # Fast path
        ASKAP_ROOT = os.environ["ASKAP_ROOT"]
        if os.path.isdir(ASKAP_ROOT + '/' + testdir):
            return True

        # Fallback to slower check. ASKAP_ROOT might not be the repository root
        (stdout, stderr, returncode) = runcmd(probecmd, shell=False)
        return True if (returncode == 0) else False
    except:
        return False

