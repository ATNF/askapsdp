##  @file
# askapdev.rbuild module containing tools to assist with the build process.
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
#

import os
import re

def get_svn_revision():
    '''Function 'lifted' from setuptools.commands.egg_info.
    If it was a member function which was static then we could have used it
    more easily.  Instead we duplicate it here.
    '''
    maxRevision = 0
    minRevision = 1e10
    errorFlag = False
    urlre = re.compile('url="([^"]+)"')
    revre = re.compile('revision="(\d+)"')

    for base, dirs, files in os.walk(os.curdir):
        try:
            if '.svn' not in dirs:
                dirs[:] = []
                continue    # no sense walking uncontrolled subdirs
            dirs.remove('.svn')
            efh = open(os.path.join(base, '.svn', 'entries'))
            data = efh.read()
            efh.close()

            if (data.startswith('8') or data.startswith('9') or
               data.startswith('10') or data.startswith('11') or
               data.startswith('12')):
                data = map(str.splitlines, data.split('\n\x0c\n'))
                del data[0][0]  # get rid of the '8', '9', '10'
                dirurl = data[0][3]
                dirrev = int(data[0][2])
                maxlocalrev = max([int(d[2] or dirrev) for d in data if len(d)>9])
                minlocalrev = min([int(d[2] or dirrev) for d in data if len(d)>9])
            elif data.startswith('<?xml'):
                dirurl = urlre.search(data).group(1)    # get repository URL
                maxlocalrev = max([int(m.group(1)) for m in revre.finditer(data)])
                minlocalrev = min([int(m.group(1)) for m in revre.finditer(data)])
            else:
                from warnings import warn
                warn("unrecognized .svn/entries format; skipping "+base)
                dirs[:] = []
                continue
            if base == os.curdir:
                base_url = dirurl+'/'   # save the root url
            elif not dirurl.startswith(base_url):
                dirs[:] = []
                continue    # not part of the same svn tree, skip it
            maxRevision = max(maxRevision, maxlocalrev)
            minRevision = min(minRevision, minlocalrev)
        except:
            # Don't break if something goes wrong!
            errorFlag = True

    if maxRevision != minRevision:
        revision = "[%s-%s]" % (str(minRevision), str(maxRevision))
        if (maxRevision+minRevision) == minRevision:
            revision = "nonsvn"
    else:
        revision = str(maxRevision)
    if errorFlag:
        revision += "ERRORS"
    return revision
