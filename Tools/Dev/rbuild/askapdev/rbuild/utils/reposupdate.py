# Copyright (c) 2006-2012 CSIRO
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
#  @author Malte Marquarding <Malte.Marquarding@csiro.au>

import os

from runcmd import runcmd
from get_vcs_type import is_svn
from q_print import q_print
from ..exceptions import BuildError

ASKAP_ROOT = os.environ["ASKAP_ROOT"]

def _is_dir_in_repo(path):
    '''Does the directory path exist in the subversion repository?
    :param path: relative (to ASKAP_ROOT) path to directory.
    '''
    comm = 'svn info %s | grep URL' % ASKAP_ROOT
    (stdout, stderr, returncode) = runcmd(comm, shell=True)

    if returncode:
        raise BuildError(stderr)

    url = stdout.split(': ')[1].strip()

    # svn 1.5 bug, returncode is 0 on failure, so grep for known label.
    comm = 'svn info %s/%s 2>&1 | grep "Node Kind:"' % (url, path)
    (stdout, stderr, returncode) = runcmd(comm, shell=True)
    if not returncode: # i.e. grep found "Node Kind:" so is in repository.
        return True

    # check if there is an svn:external on parent dir.
    parentpath = os.path.split(path)[0]
    comm = "svn propget svn:externals %s/%s" % (url, parentpath)  
    (stdout, stderr, returncode) = runcmd(comm, shell=True)        
    if stdout: # should be local directory name and remote repository path
        return True

    # Nothing found so must be local
    print 'warn: %s is a local (uncommitted) directory' % path
    return False

        
def update_command(dirpath, recursive=False, extraopts=""):
    '''execute an svn up command
 
    :param dirpath: The path to update
    :param recursive: Do a recursive update
    :param extraopts: extra options to svn command
    '''
    ropt = "-N"
    if recursive:
        ropt = ""
    comm = "svn up --non-interactive %s %s %s" % (ropt, extraopts, dirpath)
    cdir = os.path.abspath(os.curdir)
    os.chdir(ASKAP_ROOT) # Do update in root directory XXX Why?
    (stdout, stderr, returncode) = runcmd(comm, shell=True)

    if returncode:
        raise BuildError(stderr)

    if stdout.startswith("At "):
        got_updates = False
    else:
        got_updates = True
        q_print(stdout.rstrip())

    os.chdir(cdir)
    return got_updates


def update_tree(dirpath, quiet=False):
    '''svn update a (directory) path in the repository.
    This will checkout parent directories as required.

    :param dirpath: The repository dirpath to update.
    :param quiet: suppress stdout output
    '''
    if not is_svn():
        return

    rpath = None
    if not quiet:
        if dirpath.find(ASKAP_ROOT) == 0: # start of string
            rpath = os.path.relpath(dirpath, ASKAP_ROOT)
        else:
            rpath = dirpath
        print "info: Updating '%s'" % rpath

    if os.path.exists(dirpath):
        tree_updated = update_command(dirpath, recursive=True)
    elif _is_dir_in_repo(rpath):
        pathelems = dirpath.split(os.path.sep)
        # get the directory to check out recursively
        pkgdir = pathelems.pop(-1)
        pathvisited = ""

        for pth in pathelems:
            pathvisited += pth + os.path.sep
            fullpath = os.path.join(ASKAP_ROOT, pathvisited)
            if not os.path.exists(fullpath):
                tree_updated = update_command(pathvisited)
            # This is a quick way of seeing if svn:external is defined
            # i.e. svn proget svn:externals 
            # These directories need to be svn updated with --depth=infinity
            comm = "svn propget svn:externals %s" % fullpath 
            (stdout, stderr, returncode) = runcmd(comm, shell=True)
            if len(stdout) > 0:
                update_command(pathvisited, extraopts="--depth=infinity",
                               recursive=True)
        tree_updated = update_command(dirpath, recursive=True)
    else:
        tree_updated = False

    return tree_updated
