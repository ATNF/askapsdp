# Copyright (c) 2010 CSIRO
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
import os

from askapdev.rbuild.builders import SliceDoc

# When building a release it expects that the 'install' directory exists.
# In this package we do not actually install anything but since it is a
# dependency of other packages, the release target will try to process tjis
# package, so create an empty install directory so it does not fail.
# XXX Maybe this should just be moved into SliceDoc itself?

class Builder(SliceDoc):
    def _install(self):
        if not os.path.exists(self._installdir):
            os.makedirs(self._installdir)

builder = Builder(".")
builder.build()
