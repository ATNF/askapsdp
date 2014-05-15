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

#
# XXX The following should eventually move to a central configuration file
# that should be imported.
#
PKG_INFO = 'package.info'
PKG_INFO_DEFAULT =  {
    # A single directory path relative to the install directory.
    'bindir':  'bin',
    'distdir': 'dist',
    'incdir':  'include',
    'libdir':  'lib',
    # Space separated lists. XXX Default should be '[]'?
    'defs' :    None,
    'env':      None,
    'jars':     None,
    'libs':     None,
    # Define a single python module name/version. e.g. pymodule=numpy==1.2.0 
    'pymodule': None,
    }
PKG_INFO_LIST_KEYS = ['defs', 'env', 'jars', 'libs']


def get_pkginfo(packagedir):
    '''
    The format of package info entries are::
    key=value
    key=value1 value2 ... valueN
    key=value==version

    :param packagedir: the package directory which contains the info file.
    :returns: dictionary with keys/values based on info file entries.

    .. note:: Not currently used.  For ongoing dependency work see :issue:`331`:
    '''
    info = PKG_INFO_DEFAULT
    infofile = os.path.join(packagedir, PKG_INFO)

    if os.path.exists(infofile):
        f = file(infofile)
        for line in f.readlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if '=' in line:
                key, value = line.split("=", 1)
                key = key.strip()
                value = value.strip()
                if key in info.keys():
                    if key in PKG_INFO_LIST_KEYS:
                        info[key] = value.split()
                    else:
                        info[key] = value
                else:
                    q_print('warn: invalid key in %s\n      %s' % (PKG_INFO, line))
            else:
                q_print('warn: invalid line in %s\n      %s' % (PKG_INFO, line))
        f.close()
    return info


if __name__ == '__main__':
    import sys
    print get_pkginfo(sys.argv[1])
