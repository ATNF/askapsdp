## Package for various utility functions to execute build and shell commands
#
# @copyright (c) 2010 CSIRO
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
# @author Tony Maher <Tony.Maher@csiro.au>
#
import re
from in_code_tree import in_code_tree

from runcmd import runcmd

valid = re.compile('(.*\.(c|cc|h|py|tmpl|xml)|dependencies\..*|package.info)',
                   re.IGNORECASE)

def get_svn_files_list():
    '''Return the list of subversion files likely to be used in the build
    process.  Any files in the 'files*' directories are added via the
    package build.py files or the common builder.py file, and these rely on
    builder.py to add these when generating package signature.
    '''
    cmd = 'svn info --depth infinity'
    ilist = []

    for line in runcmd(cmd)[0].split('\n'):
        if line.startswith('Path:'):
            ilist.append(line.split(':')[1].strip())

    flist = []

    for item in ilist:
        if valid.match(item) and not item.startswith('files'):
            flist.append(item)

    return flist
