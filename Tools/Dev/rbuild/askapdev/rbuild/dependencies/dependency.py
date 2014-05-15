## @file
#  Module to gather dependency information for ASKAP packages
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
#  @author Malte Marquarding <malte.marquarding@csiro.au>
#
import glob
import os
import socket

import askapdev.rbuild.utils as utils
from ..exceptions import BuildError
from ordereddict import OrderedDict


## An object to hold information about a dependency tree for a nominated package
#  This package can be added manually, or if None specified, the information
#  will be taken from a file 'dependencies.{default,hostname}' in the current
#  directory.
#
# Example:
#  @code
#  f = file("dependencies.default")
#  f.write("numpy=3rdParty/numpy/numpy-1.0.2;==1.0.2\n")
#  f.close()
#  from askapdev.rbuild.dependencies import Dependency
#  dep = Dependency()
#  dep.add_package()
#  @endcode
#

class Dependency:
    ## Construct an empty dependency tree
    #  @param self the object reference
    #  @param silent  minimal feedback
    #  @param autobuild  warn rather than fail on multiple version dependnecies. XXX
    def __init__(self, silent=True, autobuild=False):
        ## The ASKAP top-level directory
        self.ASKAPROOT = os.environ.get("ASKAP_ROOT")
        if self.ASKAPROOT is None:
            msg = "ASKAP_ROOT environment variable is not defined"
            raise BuildError(msg)
        #
        self.DEPFILE = "dependencies" # The basename of the dependency file
        self.INSTALL_SUBDIR = "install"
        self._deps = OrderedDict()
        #
        self._bindirs = []
        self._incdirs = []
        self._libdirs = []
        self._rootdirs = []
        #
        self._cppflags = []  # XXX "defs" in package.info. LOFAR/log4cxx
        #
        self._env = []
        self._jars = []
        self._libs = []
        self._packages = []
        #
        self._ldlibpath = ""
        self._pypath = ""
        #
        self._autobuild = autobuild
        self._silent = silent   # mimimal output
        self.selfupdate = False # should object request updates from svn


    def q_print(self, msg):
        if self._silent:
            return
        utils.q_print(msg)


    ## Get the path of the specified dependency package
    # @param self the current object
    # @param key the label of the package dependency
    # @return the path (relative to ASKAP_ROOT) to the package
    def get_dep_path(self, key):
        return self._deps[key]["path"]


    # Used by "in" test.
    # object.__contains__(self, item)
    #
    # Called to implement membership test operators. Should return true if item
    # is in self, false otherwise. For mapping objects, this should consider
    # the keys of the mapping rather than the values or the key-item pairs.
    #
    # For objects that do not define __contains__(), the membership test first
    # tries iteration via __iter__(), then the old sequence iteration protocol
    # via __getitem__(), see this section in the language reference.
    #
    # http://docs.python.org/reference/datamodel.html

    def __contains__(self, key):
        return self._deps.has_key(key)


    ## Get the absolute path to the dependency packages installed location
    # @param self the current object
    # @param key the label of the package dependency
    # @return the absolute path to the package installed location
    def get_install_path(self, key):
        rel_path  = self._deps[key]["path"]
        full_path = os.path.join(self.ASKAPROOT, rel_path, self.INSTALL_SUBDIR)
        return os.path.abspath(full_path)

    def get_path(self):
        return os.path.pathsep.join(self._bindirs)

    ## Get the CPPFLAGS retrieved in the dependency analysis
    #  @param self the object reference
    #  @return a list of library names
    def get_libs(self, mapped=False):
        if mapped:
            return self._libs[:]
        else:
            return [ m[0] for m in self._libs ]


    ## Get the environment variables retrieved in the dependency analysis
    #  @param self the object reference
    #  @return a dictionary of ENVVAR => value pairs
    def get_env(self):
        return dict([i.split("=") for i in self._env])

    ## Get the the java classpath for the depencies
    #  @param self the object reference
    #  @return a classpath string of the form x/y/z.jar:a/b/c.jar
    def get_classpath(self):
        return os.path.pathsep.join(self._jars)

    ## Get the root directories of the tags retrieved in the dependency analysis
    #  @param self the object reference
    #  @return a list of directory names
    def get_rootdirs(self, mapped=False): # XXX used in ant.py builder with mapped=true.
        if mapped:
            return [ (k, os.path.join( self.ASKAPROOT, v['path'])) \
                for k,v in self._deps.iteritems()]
        return self._rootdirs[:]


    ## Get the LIBRARY directories retrieved in the dependency analysis
    #  @param self the object reference
    #  @param mapped return directory tuples (rootdir, libdir)
    #  @return a list of library directories or tuples of rootdirs and libdirs
    def get_librarydirs(self, mapped=False):
        if mapped:
            return self._libdirs[:]
        else:
            return [ m[0] for m in self._libdirs ]


    ## Get the LD_LIBRARY_PATH accumulated in the dependency analysis
    #  @param self the object reference
    #  @return a string representing the LD_LIBRARY_PATH
    def get_ld_library_path(self):
        return self._ldlibpath.strip(":")


    ## Get the INCLUDE directories retrieved in the dependency analysis
    #  @param self the object reference
    #  @return a list of header file directories
    def get_includedirs(self):
        return self._incdirs[:]


    ## Get the CPPFLAGS retrieved in the dependency analysis
    #  @param self the object reference
    #  @return a list preprocessor flags
    def get_cppflags(self):
        return self._cppflags[:]


    def get_pythonpath(self):
        return self._pypath.strip(":")


    ## Get a list of doxygen tag files in the dependencies. This is used for
    #  cross-referencing the documentation
    #  @todo Re-enable: This has been disabled until it is working for python
    #  @param self the object reference
    #  @return a list of TAGFILES entries
    # XXX used only in scons_tools/askap_package.py
    def get_tagfiles(self):
        tagfiles = []
        for pth in self._rootdirs:
            tagname = utils.tag_name(pth)
            tagpath = os.path.join(pth, tagname)
            if os.path.exists(tagpath):
                tagfiles.append('"%s=%s/html"' % (tagpath, pth) )
        return tagfiles


    def _get_dependencies(self, package):
        codename = utils.get_platform()['codename']
        hostname = socket.gethostname().split(".")[0]

        for ext in ['default', codename, hostname]:
            if ext:
                depfile = '%s.%s' % (self.DEPFILE, ext)
                if package:
                    depfile = os.path.join(self.ASKAPROOT, package, depfile)
                if self.selfupdate:
                    # always update if it is the "root/target" package
                    basedir = os.path.split(depfile)[0] or "."
                    if not os.path.exists(basedir):
                        utils.update_tree(basedir)
                self._get_depfile(depfile)


    def _get_depfile(self, depfile, overwrite=False):
        if not os.path.exists(depfile):
            # assume no dependencies
            return
        dfh = file(depfile)
        for line in dfh.readlines():
            line = line.strip()
            if line.startswith("#"): continue
            kv = line.split("=", 1)
            if len(kv) == 2:
                key = kv[0].strip()
                value = kv[1].strip()
                # see if the file explicitly names any libs
                lspl = value.split(";")
                libs = None
                if len(lspl) > 1:
                    libs = lspl[1].strip().split()
                value = lspl[0]
                self._add_dependency(key, value, libs, overwrite)
                if not value.startswith("/"):
                    # recurse into ASKAP dependencies
                    # otherwise just move on as we specified system dependency
                    # which will not have a dependency file
                    self._packages.append(value)
                    self._get_dependencies(value)

        dfh.close()


    def _get_info(self, packagedir):
        info =  {
                # A single directory path relative to the install directory.
                'bindir':  'bin',
                'distdir': 'dist',
                'incdir':  'include',
                'libdir':  'lib',
                # Space separated lists. XXX Default should be '[]'?
                'defs' :    None,
                'env':      None,
                'jars':     None,
                'libs':     None,
                # Define a single python module name and version.
                # e.g. pymodule=numpy==1.2.0
                'pymodule': None,
                }
        sslists = ['defs', 'env', 'jars', 'libs']
        infofile = os.path.join(packagedir, 'package.info')

        if os.path.exists(infofile):
            f = file(infofile)
            for line in f.readlines():
                line = line.strip()
                if line.startswith("#"): continue
                kv = line.split("=", 1)
                if len(kv) == 2:
                    key = kv[0].strip()
                    value = kv[1].strip()
                    if key in info.keys():
                        if key in sslists:
                            info[key] = value.split()
                        else:
                            info[key] = value
            f.close()
        return info


    def _add_dependency(self, key, value, libs, overwrite=False):
        if self._deps.has_key(key):
            # deal with potential symbolic links for 'default' packages
            paths = [self._deps[key]["path"], value]
            outpaths = []
            for pth in paths:
                if not pth.startswith("/"):
                    pth = os.path.join(os.environ["ASKAP_ROOT"], pth)
                pth = os.path.realpath(pth)
                outpaths.append(pth)
            if outpaths[0] == outpaths[1]:
                if libs:
                    if self._deps[key]["libs"] is not None:
                        # prepend the libs
                        self._deps[key]["libs"] = libs + self._deps[key]["libs"]
                    else:
                        self._deps[key]["libs"] = libs
                    self._deps.toend(key)
                else:
                    # another dependency, so move it to the end, so link
                    # order is correct
                    self._deps.toend(key)
                return
            else:
                if overwrite:
                    self._deps[key]["path"] = value
                    self.q_print("info: Overwriting default package dependency '%s' with host specific package (from %s)" % (key, value) )
                elif self._autobuild: # XXX maybe a mistake?
                    self.q_print("warn: Possible multiple version dependency \n\
                    %s != %s" % (self._deps[key]["path"], value))

                else:
                    raise BuildError("Multiple version dependency \n\
                    %s != %s" % (self._deps[key]["path"], value))
        else:
            self.q_print("info: Adding package dependency '%s' (from %s)" %
                          (key, value))
            # now update the dependency itself
            # XXX only used in Tools/scons_tools/askap_package.py
            if self.selfupdate:
                utils.update_tree(value)
            self._deps[key] = {"path": value, "libs": libs}


    def _remove_duplicates(self, values):
        # find unique elements
        libs = [v[0] for v in values]
        for k in set(libs):
            # remove all but last duplicate entry
            while libs.count(k) > 1:
                idx = libs.index(k)
                libs.pop(idx)
                values.pop(idx)

    ## Add a ThirdPartyLibrary or ASKAP package to the environment
    #  This will add the package path in ASKAP_ROOT
    #  @param self the object reference
    #  @param pkgname The name of the package as in the repository, e.g.
    #  lapack. Default None means that this is defined in local
    #  dependencies.xyz
    #  @param tag The location of the package, e.g.
    #  3rdParty/lapack-3.1.1/lapack-3.1.1
    #  @param libs The name of the libraries to link against,
    #  default None is the same as the pkgname
    #  @param libdir The location of the library dir relative to the package,
    #  default None which will use settings in the package.info file
    #  @param incdir The location of the include dir relative to the package,
    #  default None which will use settings in the package.info file
    #  @param pymodule the 'require' statement to specify this dependency
    #  statement, e.g. "askap.loghandlers==current"
    def add_package(self, pkgname=None, tag=None,
                    libs=None, libdir=None, incdir=None, bindir=None,
                    pymodule=None):
        self._deps = OrderedDict()

        if pkgname:
            if not tag:
                BuildError("No tag specified")
            if self.selfupdate:
                #if not os.path.exists(tag):
                utils.update_tree(tag)
            self._add_path(pkgname, self.ASKAPROOT, tag, libs, libdir,
                          incdir, bindir, pymodule)
            self.q_print("info: Adding package '%s'" % pkgname)

        if tag:
            tag = os.path.join(self.ASKAPROOT, tag)

        self._get_dependencies(tag)

        parent = ''
        for key, value in self._deps.iteritems():
            self._add_path(key, self.ASKAPROOT, value["path"],
                           libs=value["libs"], parent=parent)
            parent = value["path"]


    # Add a ASKAP repository path to the environment
    # This sets up LIBPATH and CPPPATH
    def _add_path(self, pkgname, root, tag, parent='', libs=None,
                 libdir=None, incdir=None, bindir=None,
                 pymodule=None):
        loc = None

        if tag.startswith("/"): # external package
            loc = tag
        else:                   # ASKAP package or 3rdParty library
            loc = os.path.join(root, tag)

        rloc = os.path.relpath(loc, self.ASKAPROOT)
        if not os.path.exists(loc):
            raise BuildError("Dependency directory '%s' does not exist (requested by %s)." % (rloc,parent))

        self._rootdirs += [loc]
        info = self._get_info(loc) # get optional package info
        idir = os.path.join(loc, self.INSTALL_SUBDIR) # actual installion.

        if not bindir: # add bin directory
            bindir = info["bindir"]

        if bindir: # None means disabled in info file
            pth = os.path.join(idir, bindir)
            if os.path.exists(pth):
                self._bindirs += [pth]

        if not incdir: # add include directory
            incdir = info["incdir"]

        if incdir: # None means disabled in info file
            pth = os.path.join(idir, incdir)
            if not os.path.exists(pth):
                if not pymodule:
                    self.q_print("warn: incdir '%s' does not exist." % pth)
            else:
                self._incdirs += [pth]

        if not libdir: # add library directory
            libdir = info["libdir"]

        if libdir: # None means disabled in info file
            pth = os.path.join(idir, libdir)
            if not os.path.exists(pth):
                if not pymodule:
                    self.q_print("warn: libdir '%s' does not exist." % pth)
            else:
                self._ldlibpath += os.path.pathsep+pth
                self._libdirs += [(pth, idir)]

        libs = libs or info["libs"]
        addlibs = True
        
        if isinstance(libs, list) and len(libs) == 0:
            addlibs = False

        libs = libs or pkgname

        if not isinstance(libs, list):
            libs = [libs]

        if addlibs: # only add lib if it's not a python module
            nlibs = []
            for lib in libs:
                instdir = idir
                if not glob.glob("{0}/lib{1}*".format(os.path.join(idir, 
                                                                   libdir),
                                                      lib)):
                    instdir = ""
                nlibs.append((lib, instdir))
            self._libs += nlibs
            libs = self._libs[:] # copy
            self._remove_duplicates(libs)
            self._libs = libs

        if info["defs"]: # add package defines
            self._cppflags += info["defs"]

        if info["env"]: # add environment variables
            self._env += info["env"]

        # check whether it is python, i.e. pymodule entry in package.info
        if not pymodule:
            pymodule = info["pymodule"]
        if pymodule:
            pth = os.path.join(idir, libdir, utils.get_site_dir())
            if self._pypath.find(pth) < 1:
                self._pypath = os.path.pathsep.join([pth, self._pypath])
        
        if info["jars"]:
            pth = os.path.join(idir, libdir)
            if not os.path.exists(pth):
                if not pymodule:
                    self.q_print("warn: libdir '%s' does not exist." % pth)
            for jar in info["jars"]:
                jar = os.path.join(pth, jar)
                if jar not in self._jars:
                    self._jars.append(jar)
