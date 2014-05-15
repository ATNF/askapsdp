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

import platform

## Return a dictionary describing the platform.
def get_platform():
    '''
    The function returns a dictionary with keys::
    
        ['hostname', 'system', 'distribution', 'version', 'tversion',
         'architecture', 'kernel', 'tkernel',]
    For Debain systems it also returns the codename for the release.

    .. note::

        The 'tversion' and 'tkernel' are tuple representations, rather than
        string.

    Example::
    
        python -c 'from askapdev.rbuild.utils import get_platform; print get_platform()'
        {'kernel': '2.6.26', 'version': '5.0', 'architecture': '64bit', 'tversion': ('5', '0', '9'),
        'distribution': 'debian', 'hostname': 'delphinus', 'tkernel': ['2', '6', '26'], 'system': 'Linux'}

    .. note::

        See Issue :issue:`2420` for comments on why things done the way they are.
    '''
    system, hostname, release, uname, machine, processor = platform.uname()
    architecture = platform.architecture()[0]
    distribution = ''   # Most platforms are defined by 'system' only.
    codename = ''       # Stupid codenames for MacOSX and Debian.
    kernel = release.split('-')[0]
    tkernel = kernel.split(".")

    if system == 'Darwin':
        version = platform.mac_ver()[0]
    elif system == 'Linux':
        distribution, version = platform.dist()[0:2]
    elif system == 'FreeBSD':
        version = kernel # '8.0-RELEASE-p2'
    else:
        version = '' # Handle platforms we do not yet cover.

    if hasattr(version, 'split'):
        version2 = '.'.join(version.split('.')[0:2])
        tversion = tuple([i for i in version.split('.')])
    else:
        version2 = ''
        tversion = tuple()
        
    if distribution == 'debian':
        v = float(version2)
        if   v >= 8: codename = 'jessie'
        elif v >= 7: codename = 'wheezy'
        elif v >= 6: codename = 'squeeze'
        elif v >= 5: codename = 'lenny'
        elif v >= 4: codename = 'etch'

    if system == 'Darwin':
        if   version2 == '10.9': codename = 'mavericks'
        elif version2 == '10.8': codename = 'mountain_lion'
        elif version2 == '10.7': codename = 'lion'
        elif version2 == '10.6': codename = 'snow_leopard'
        elif version2 == '10.5': codename = 'leopard'
        elif version2 == '10.4': codename = 'tiger'


    return {'hostname'     : hostname,
            'system'       : system,
            'distribution' : distribution,
            'version'      : version2,
            'tversion'     : tversion,
            'architecture' : architecture,
            'kernel'       : kernel,
            'tkernel'      : tkernel,
            'codename'     : codename,
           }


if __name__ == '__main__':
    print get_platform()
