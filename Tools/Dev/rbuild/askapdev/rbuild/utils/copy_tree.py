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

import errno
import fnmatch
import os
import shutil
import sys

from q_print import q_print
from ..exceptions import BuildError

OPTIONS = sys.argv[1:]


def copy_tree(src, dst, pattern=None, overwrite=False, symlinks=True, mode=0755):
    '''
    Recursively copy a directory to a target directory.
    For creating releases, we need to be able to copy multiple subtrees
    of bin, lib etc to a central release point.
    Existing python tools for copy such as shutils.copytree, requires that the
    target directory does not exist so cannot call it multiple times.
    This code is based on example code for copytree in Python shutil module
    documentation section 11.10.1 Example.

    :param src:       The source directory.
    :type src:        str
    :param dst:       The target directory (it will be created if it
                      does not exist).
    :type dst:        str
    :param pattern:   Match only files of this pattern (default is None so
                      all files get copied).  Directories are always copied.
    :type pattern:    str
    :param overwrite: controls whether to overwrite existing files
                      (default is False).
    :type overwrite:  bool
    :param symlinks:  controls whether symlinks are to be created in target or
                      ignored completely (default is True).
    :type symlinks:   bool
    :param mode:      mode for created directories (default 0755).
    :type mode:       stat
    :return:          None
    '''
    if os.path.isfile(src):
        ddir = os.path.split(dst)[0]
        if ddir and not os.path.exists(ddir):
            os.makedirs(ddir)
        shutil.copy(src, dst)
        return

    # Try to create the target directory.
    # If it already exists and is a directory then thats ok, and we will
    # try to use it as is. For all other errors re-raise the exception.
    try:
        os.makedirs(dst, mode)
    except OSError, (numerr, strerr):
        if not (errno.errorcode[numerr] == 'EEXIST' and os.path.isdir(dst)):
            raise BuildError("Couldn't create directory %s" % dst)

    for name in os.listdir(src):
        if name in ['.svn', 'CVS', 'RCS']:
            continue

        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)

        if os.path.isdir(srcname):
            copy_tree(srcname, dstname, overwrite=overwrite)
        elif pattern is None or fnmatch.fnmatch(name, pattern):
            path_exists = os.path.exists(dstname)
            if not path_exists or overwrite:
                if path_exists:
                    q_print("warn: Overwriting %s" % dstname)
                    os.remove(dstname)
                if os.path.islink(srcname):
                    if symlinks:
                        linkto = os.readlink(srcname)
                        try:
                            os.symlink(linkto, dstname)
                        except:
                            print >>sys.stderr,  "warn: symlink to", \
                                dstname ,"exists"
                else:
                    shutil.copy(srcname, dstname)
            else:
                q_print("warn: Have not copied %s as it would have overwrote %s" % (srcname, dstname))
