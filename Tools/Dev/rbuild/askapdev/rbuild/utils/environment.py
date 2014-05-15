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
import sys
import os
import re

from askapdev.rbuild.utils import get_site_dir

def _remove_system_dirs(path):
    '''
    Remove any system i.e. /usr/ paths from a path list.

    :param path: path list (separated bu os.path.pathsep).
    :type path:  str
    :return:     reduced path list.
    '''
    items = path.split(os.path.pathsep)
    out = [item for item in items\
           if not os.path.abspath(os.path.expandvars(item)).startswith('/usr')]
    return os.path.pathsep.join(out)


def _get_ld_prefix():
    '''
    :return: "DY" for darwin platform otherwise "".
    :rtype:  str
    '''
    return sys.platform == "darwin" and "DY" or ""


def _get_pkg_paths(prefix, infofile):
    '''
    Get package paths from info file and current package.

    :param prefix:   the path prefix
    :param infofile: the info file
    :return:         (bin_path, ld_library_path, classpath, pythonpath)
    '''
    pth = os.path.join(prefix, 'bin')
    ldpth = os.path.join(prefix, 'lib')
    clspth = None
    pypth = None

    if os.path.exists(infofile):
        for line in open(infofile, 'r'):
            line = line.strip()
            if line.startswith('#') or len(line) == 0:
                continue
            k,v = line.split("=", 1)
            if k == "bindir":
                pth = os.path.join(prefix, v)
            elif k == "libdir":
                ldpth = os.path.join(prefix, v)
            elif k == "jars":
                v = [os.path.join(ldpth, i) for i in v.split()]
                clspth = os.path.pathsep.join(v)
            elif k == "pymodule":
                pypth = os.path.join(ldpth, get_site_dir())

    return pth, ldpth, clspth, pypth


class Environment(object):
    '''
    Environment object for packages.
    '''
    def __init__(self, dep, prefix, infofile, development=True):
        '''
        :param dep:
        :param prefix:
        :param inforfile:
        :param development:

        '''
        self._dep = dep
        self._bindir, self._libdir, self._classpath, self._pypath = \
            _get_pkg_paths(prefix, infofile)
        self._variables = {}
        self._get_paths(development)
        self.ld_prefix = _get_ld_prefix()

    def __getitem__(self, k):
        return self._variables[k]

    def __iter__(self):
        yield iter(self._variables.keys())

    def items(self):
        return self._variables.items()

    def _get_path(self, path, localpath=None):
        out = path
        if localpath:
            if path:
                out = os.path.pathsep.join([localpath, path])
            else:
                out = localpath
        aroot = os.environ["ASKAP_ROOT"]
        out = out.replace(aroot, '${ASKAP_ROOT}').rstrip(os.path.pathsep)
        return out

    def replace(self, pathval, dirname):
        '''
        :param pathval: string list of paths separated by os.path.pathsep
        :type pathval:  str
        :param dirname: directory name to search for.
        '''
        path_env = []
        # rx.findall returns list of tuples,
        # (ASKAP_ROOT, dirname [+ subdirs])
        rx = re.compile('(\$\{ASKAP_ROOT\}/).+/install/(%s.*)'
                        % dirname)
        for path_elem in pathval.split(os.path.pathsep):
            path_list = rx.findall(path_elem)
            if path_list:
                mod_path_elem = "".join(path_list[0])
                if mod_path_elem not in path_env:
                    path_env.append(mod_path_elem)

        # Always add the top level bin, lib hierarchy.
        basedir = os.path.join("${ASKAP_ROOT}", dirname)
        if basedir not in path_env:
            path_env.append(basedir)
        return os.path.pathsep.join(path_env)


    def _get_paths(self, development=True):
        dirs = {'PATH' : (self._bindir, self._dep.get_path()),
                'LD_LIBRARY_PATH' : (self._libdir, self._dep.get_ld_library_path()),
                'CLASSPATH': (self._classpath, self._dep.get_classpath()),
                'PYTHONPATH': (self._pypath, self._dep.get_pythonpath()),
        }
        for k, v in self._dep.get_env().items():
            dirs[k] = (v, None)

        fulldirs = {}
        for k, v in dirs.items():
            fulldirs[k] = self._get_path(v[1], v[0])
            if not development:
                fulldirs[k] = self.replace(fulldirs[k], '')
            fulldirs[k] = _remove_system_dirs(fulldirs[k])
        self._variables.update(fulldirs)
