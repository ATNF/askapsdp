## Package for defining additional setup target command objects.
#
# Each command uses a variety of arguments. Currently these args work from
# the command line and setup.cfg file but not as additional parameters to the
# setup function call. It seems that the way that setuptools add the
# test-suite option is by actually extending the Distribution class as well
# as providing a test command. Note that where possible the arguments will
# default to useful values but it is a good idea to specify them from an SCM
# point of view.
#
# The correct usage is to import the command class into your setup.py file and
# then pass it into the setup function using eg. cmdclass = {'doc': doc}. Note
# that this is automatically done for you if you use askapdev.rbuild.setup().
#
# @copyright (c) 2006 CSIRO
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
# @author Robert Crida <robert.crida@ska.ac.za>
# @date 2006-12-14
#


from clean import clean
from doc import doc
from pylint import pylint
