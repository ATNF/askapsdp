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
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
import os.path
import datetime

from get_package_name import get_package_name
from get_svn_revision import get_svn_revision
from get_svn_branch_info import get_svn_branch_info


def get_release_version():
    '''Return the branch of the repository we are using.
    e.g. ['trunk'], ['releases', '0.3'], ['features', 'TOS', 'JC'] etc
    '''
    currentrev = get_svn_revision()
    bi = get_svn_branch_info()
    items = [get_package_name()]
    items.append("==".join(["ASKAPsoft", os.path.join(*bi)]))
    items.append("r" + currentrev)
    items.append(str(datetime.date.today()))
    return "; ".join(items)


if __name__ == '__main__':
    print get_release_version()
