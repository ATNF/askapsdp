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
import sys

from runcmd import runcmd
from q_print import q_print

def number_of_cpus():
    '''Return the number of CPUs the current machine has.'''
    try:
        if sys.platform.startswith("linux"):
            output = runcmd("cat /proc/cpuinfo")[0]
            ncpu = 0
            for line in output.split('\n'):
                if line.startswith('processor'):
                    ncpu += 1
        elif (sys.platform.startswith("darwin") or
                 sys.platform.startswith("freebsd")):
            ncpu = runcmd("sysctl -n hw.ncpu")[0]
            ncpu = int(ncpu)
        else:
            q_print("warn: Do not know how to find number of CPUs for %s platform" % sys.platform)
            ncpu = 1
    except:
        q_print("warn: Exception in finding number of CPUs, setting to 1")
        ncpu = 1
    if type(ncpu) != int: # Paranoia - want to guarantee an int return.
        q_print("warn: Problem in finding number of CPUs, setting to 1")
        ncpu = 1

    return ncpu
