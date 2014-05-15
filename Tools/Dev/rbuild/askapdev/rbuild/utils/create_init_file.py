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
from __future__ import with_statement
import os

def create_init_file(name, env):
    '''
    Create package initialization file. e.g. init_package_env.sh

    :param name: file name to be created.
    :type name: string
    :param env: the environment object.
    :type env: :class:`.Environment`
    :return: None
    '''
    aroot = os.environ["ASKAP_ROOT"]
    inittxt= """#
# ASKAP auto-generated file
#
ASKAP_ROOT=%s
export ASKAP_ROOT
""" % aroot

    vartxt = """if [ "${%(key)s}" !=  "" ]
then
    %(key)s=%(value)s:${%(key)s}
else
    %(key)s=%(value)s
fi
export %(key)s
"""

    with open(name, 'w') as initfile:
        initfile.write(inittxt)
        for k, v in env.items():
            if not v:
                continue
            if k == "LD_LIBRARY_PATH":
                k = env.ld_prefix+k
            v = v.replace(aroot, '${ASKAP_ROOT}')
            initfile.write(vartxt % { 'key': k, 'value': v } )
