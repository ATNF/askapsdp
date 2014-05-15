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

import distutils
import os

from get_python_version import get_python_version


def get_site_dir():
    '''
    :return: relative python site package directory
    :rtype: str
    '''
    return os.path.join("python%s" % get_python_version(), "site-packages")


def create_python_tree(root=None):
    '''
    Create a python tree.
    The python easyinstall does not create directories but expects them to
    exist.

    :param root: root directory for the tree (default=None meaning current
                 directory).
    :type root: str
    '''
    sp = os.path.join("lib", get_site_dir())
    for d in ["bin", sp]:
        if root:
            d = os.path.join(root, d)
        try:
            os.makedirs(d, 0755)
        except:
            pass

    # The following was added to allow building on epic cluster (Issue #5018)
    # but maybe required on other platforms.
    # Not sure about how best to do this.
    # Maybe should just create lib64 instead of lib above.
    if 'lib64' in distutils.sysconfig.get_config_var('LIBDIR'):
        curdir = os.getcwd()
        os.chdir(root)
        if not os.path.exists('lib64'):
            os.symlink('lib', 'lib64')
        os.chdir(curdir)
