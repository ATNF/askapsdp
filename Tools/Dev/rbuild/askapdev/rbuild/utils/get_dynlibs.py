## Package for various utility functions to execute build and shell commands
#
# @copyright (c) 2011 CSIRO
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

import glob
import os

from q_print import q_print
from get_platform import get_platform

def get_dynlibs(libdir):
    '''
    Return all dynamic libraries found in a directory without extension or
    the leading 'lib'.

    :param libdir: the directory to examine
    :return: list of libraries with 'lib' prefix and extension stripped.

    .. note:: Not currently used. For ongoing dependency work see :issue:`331`:
    '''
    if os.path.isdir(libdir):
        system = get_platform()['system']
        if system in ['Darwin',] :
            ext = '.dylib'
        else:
            ext = '.so'

        pattern = 'lib*' + ext + '*'

        cwd = os.getcwd()
        os.chdir(libdir)
        libs = [lib.split(ext)[0][3:] for lib in glob.glob(pattern)]
        os.chdir(cwd)
    else:
        q_print('warn: library directory %s does not exist.')
        libs = []

    return libs


if __name__ == '__main__':
    import sys
    print get_dynlibs(sys.argv[1])
