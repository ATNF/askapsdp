#
# @copyright (c) 2013 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
__all__ = ['add_debian']
import os
import stat
import pkg_resources
from .. import utils
from .naming import get_versioned_name

A_ROOT = os.getenv("ASKAP_ROOT") 

def add_debian(package):    
    """Add a debian directory to the give package"""
    odir = os.path.abspath(os.curdir)
    pdir = package
    if not pdir.startswith(os.sep):
        pdir = os.path.join(A_ROOT, pdir)

    debdir = os.path.join(pdir, "debian")
    if os.path.exists(debdir):
        print "info:", package, "debianised already"
        return
    else:
        os.mkdir(debdir)
    name, version = get_versioned_name(package)
    if name is None or version is None:
        print "warn: Can't determine debian package name or version for", \
            package
        return
    d = { 'package': name, 'version' : version }
    for f in pkg_resources.resource_listdir(__name__, 'data'):
        data = pkg_resources.resource_string(__name__, 
                                             os.path.join("data", f))
        if f in ['changelog', 'control', 'rules']:
            for k,v in d.items():
                data = data.replace('@@@'+k+'@@@', v)
        debf = os.path.join(debdir, f)
        with open(debf, "w") as of:
            of.write(data)
        if "rules" == f:
            st = os.stat(debf)
            os.chmod(debf, st.st_mode | stat.S_IEXEC)
    o,e,rc = utils.runcmd("svn add %s" % debdir)
    if rc != 0:
        raise OSError(e)

    utils.q_print("info: Created debian package '%s (%s)' "
                  "infrastructure for %s"  % (name, version, package))
