# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
# @copyright (c) 2009 CSIRO
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

from builder import Builder
from ant import Ant
from autotools import Autotools
from scons import Scons
from setuptools import Setuptools
from epics import Epics
from adbe import Adbe
from virtual import Virtual
from data import Data
from slicedoc import SliceDoc
from qt import Qt
from metapackage import MetaPackage
from cmake import CMake

from askapdev.rbuild.utils import get_python_version

## For release require an initialisation file.
# Expect only shell (bash/ksh/sh) will be used for production.
#
init = '''#ASKAPsoft release initialization file.
#
# There is no way to determine the location of this initialization file
# when it is sourced (i.e. '. initaskap.sh').  However, if the script is
# run (i.e. 'sh initaskap.sh') then you can determine the location.
# After installation into the target machine, run the script once
# which will set the TOP level directory location.  Then this file can
# be sourced for initialization purposes.
#
if [[ "`basename $0`" = "initaskap.sh" ]] ; then
    echo 'Determining installation directory and setting the $TOP variable to'
    if [[ $0 == '/'* ]]; then
        loc="`dirname $0`"
    else
        loc="`pwd`"/"`dirname $0`"
    fi
    echo $loc
    sed -i -e "s#\(^TOP=\).*#\\1${loc}#" $0
    exit
fi

# Normal initialization.

TOP=@@@TOP@@@

LD_LIBRARY_PATH="${TOP}/lib:${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH

PYTHONPATH="${TOP}/lib/python%s/site-packages:${PYTHONPATH}"
export PYTHONPATH

PATH="${TOP}/bin:${PATH}"
export PATH

MANPATH="${TOP}/man:${MANPATH}"
export MANPATH
''' % get_python_version()
