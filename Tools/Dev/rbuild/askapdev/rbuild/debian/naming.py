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
__all__ = ["to_debian_name", "depends_list", "get_versioned_name", "get_package_revision"]

import os
import re
from ..dependencies.depends import Depends
from .. import utils

def to_debian_name(package):
    """Get the debian package name for the given ASKAPsoft package"""
    r = package.lower()
    #3rdParty
    if r.startswith("3rdparty"):
        elem = r.split("/")[1:]
        if elem[0] == 'epics':
            # epics
            if elem[-1] == "base":
                r = "-".join([elem[0], elem[-1], elem[1]])
            else:
                # normal packages
                r = elem[-1]
        # combined packages, e.g. LOFAR
        elif len(elem) > 2:
            r = "-".join([elem[0], elem[-1]])
        else:
            r = elem[-1]
        # Prefix thirdparty packages with 'askap-dep'
        r = "-".join(["askap", "dep", r])
    # Code tree
    else:
        r = r.replace("code/","")
        r = r.replace("/current", "")
        if r.startswith("base"):
            r = r.replace("base/", "")
            r = "-".join([r, "base"])
        elif r.startswith("components"):
            r = r.replace("components/", "")
            r = r.replace("/", "-")
        elif r.startswith("interfaces"):
            r = r.replace("/", "-")
        if not r.startswith("askap"):
            r = "-".join(["askap", r])
    return r.replace("_", "-")


def depends_list(pkg_path):
    """Get a list of debian package dependencies for the given ASKAPsoft 
    package"""
    d = Depends(rootpkgdir=pkg_path)
    deps = []
    for name in d.get_explicit():
        nodeb = os.path.join(os.getenv("ASKAP_ROOT"), name, "NO_DEBIAN")
        if os.path.exists(nodeb):
            utils.q_print("Ignoring package {0}".format(name))
            continue
        dep = get_versioned_name(os.path.normpath(name)) 
        if "3rdParty" in name:
            deps.append("{0} (>={1}), {0} (<={1}patch99)".format(*dep))
        else:
            deps.append(dep[0])
    return ", ".join(deps)

def get_versioned_name(package):
    """Get a [name, version] tuple for a given ASKAPsoft package"""
    a_root = os.getenv("ASKAP_ROOT")
    if a_root in package:
        package = package.replace(a_root+"/", "")
    rxrev = re.compile(r"^r\d+$")
    rxvers = re.compile(r"^r?(\d+(?:\.\d+)+[a-zA-Z]?)$")
    versioned = []
    deb = to_debian_name(package)
    if "3rdParty" in package:
        elem = deb.rsplit("-", 1)
        m = rxvers.search(elem[-1])
        # regular package version e.g. foo-1.2.4
        if m:                
            versioned = [elem[0], m.group(0)]
        else:
            # svn rev e.g. foo-r234
            if rxrev.search(elem[-1]):
                versioned = (elem[0], elem[-1].lstrip("r"))
            else:
                elem = deb.rsplit("_", 1)
                # underscored version e.g. eg_1234
                if len(elem) > 1 and re.search('\d+',elem[-1]):
                    versioned = elem[:]
                else:
                    m = re.search("\d", deb)
                    # epics extension type version e.g. foo2-4, bar-1-2
                    if m:
                        idx = m.start()
                        vstr =  deb[idx:].replace("-",".")
                        if deb[idx-1] == "-":
                            versioned = [deb[:idx-1], vstr]
                        else:
                            versioned = [deb[:idx], vstr]

                    # not handled
                    else:
                        versioned = [None, None]
    else:
        pth = os.path.join(a_root, package)
        rev = get_package_revision(pth)
        versioned = [deb, "%s~tos1.0" %rev]        
    return versioned


def get_package_revision(pth):
    cmd = "svn info %s |grep Rev: |awk '{print $NF}'" % pth
    out = utils.runcmd(cmd, shell=True)
    if out[-1] != 0 or len(out[0]) == 0:
        raise OSError(out[1])
    vers = int(out[0].strip())
    procf = os.path.join(pth, "debian", "procServ.template")
    if os.path.exists(procf):
        cmd = "svn info %s |grep Rev: |awk '{print $NF}'" % procf
        out = utils.runcmd(cmd, shell=True)        
        vers = max(vers, int(out[0].strip()))
    return str(vers)

