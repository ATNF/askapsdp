# Copyright (c) 2009 CSIRO
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

import os
import sys
import glob
import string
import datetime
from SCons.Defaults import Mkdir
from askapdev.rbuild.dependencies import Dependency
import askapdev.rbuild.utils as utils

def q_print(msg):
    if not "-Q" in sys.argv[1:]:
        print msg

def generate(env):
    class AskapPackage:
        def __init__(self, name, builddir="build"):
            self._name = "askap_"+name
            self._builddir = builddir
            self._sources = []
            self._headers = []
            self._libs = []
            self._apps = []
            env["PACKAGE"] = name
            env.PrependUnique(CPPPATH=["#"])
            self._subpackages = []
            self._tprogs = []
            self.rt = []
            self.ldpathvar = "LD_LIBRARY_PATH"
            if env["PLATFORM"] == "darwin":
                self.ldpathvar = "DY"+self.ldpathvar
            self._CreateConfig()
            env.AddAskapPackage()
            env.Alias("slice", None)
            self._shared = True
            self._static = False

        def set_shared(self, value):
            self._shared = value

        def get_shared(self):
            return self._shared

        build_shared = property(get_shared, set_shared)

        def set_static(self, value):
            self._static = value

        def get_static(self):
            return self._static

        build_static = property(get_static, set_static)


        def AddLib(self, lib, append=True):
            add = env.AppendUnique
            if not append:
                add = env.PrependUnique
            add(LIBS=[lib])

        def AddSubPackage(self, subdir):
            # get source
            if not os.path.isdir(subdir):
                env.Exit(1)
            for pth, d, files in os.walk(subdir):
                self._sources += env.Glob("%s/[A-Z]*.cc" % pth, strings=True)
                self._sources += env.Glob("%s/[a-z]*.c" % pth, strings=True)
                self._headers += env.Glob("%s/[A-Z]*.tcc" % pth, strings=True)
                self._headers += env.Glob("%s/[a-zA-Z]*.h" % pth, strings=True)
            self._subpackages.append(subdir)

        def AddInterfaces(self, intfiles, subpkg, intdir=None):
            iceenv = env.Clone()
            ice = iceenv.get("ICE_HOME", None)
            if not ice:
                print "Ice not found"
                iceenv.Exit(1)
            if not subpkg in self._subpackages:
                print "subpackage not found"
                iceenv.Exit(1)
            if isinstance(intfiles, str):
                intfiles = [intfiles]
            if intdir is None:
                intdir = iceenv["INTERFACE_HOME"]
            else:
                intdir = os.path.abspath(intdir)
            intfiles = [os.path.join(intdir, ifile) for ifile in intfiles ]
            iceenv["ENV"][self.ldpathvar] = iceenv["ASKAP_LIBRARY_PATH"]
            iceenv["SLICEOUT"] = subpkg
            iceenv["SLICEINCLUDE"] = subpkg
            # Need to add include directory of slice generated header file
            # Currently only needed under linux; darwin seems to not use
            # include in header file.
            env.PrependUnique(CPPPATH=[os.path.join("#",subpkg)])
            self._slice = []
            for intf in intfiles:
                tgt = iceenv.Slice(source=intf)
                self._slice.append(str(tgt[0]))
                iceenv.Alias("slice", tgt)
            # prepend source so the cc/h files are generated before they can be
            # used
            self._sources = self._slice+self._sources

        def FuncTests(self):
            if os.path.exists("functests"):
                functestenv = env.Clone()
                functestenv.PrependUnique(LIBS=[self._name])
                functestenv.PrependUnique(LIBPATH=["#"])
                functestenv.SConscript("functests/SConscript",
                                       exports='functestenv')

        def AddApps(self, appdir="apps"):
            if not os.path.exists(appdir):
                return False
            appenv = env.Clone()
            appfiles = glob.glob("%s/*.cc" % appdir)
            appfiles += glob.glob("%s/*.c" % appdir)
            def createrun(target, env):
                filepth = str(target[0]).split("/")
                fname = filepth[-1]
                pth = string.join(filepth[:-1], "/")
                outf = "%s/%s.sh" % (pth, fname)
                f = open(outf, 'w')
                f.write(env['RUNTXT'])
                f.write("\n")
                tmpl = os.path.join(pth,fname+".run")
                if os.path.exists(tmpl):
                    f.write(file(tmpl,"r").read())
                else:
                    prog = os.path.abspath(str(target[0]))
                    prog = prog.replace(env["ASKAP_ROOT"], "${ASKAP_ROOT}")
                    f.write('exec %s "$@"' % prog)
                    f.write("\n")
                f.close()
                os.chmod(outf, 0755)
                se = appenv.SideEffect(outf, target)
                appenv.Clean("tidy", se)
                return 0

            ldpath = appenv["ASKAP_LIBRARY_PATH"].replace(env["ASKAP_ROOT"],
                                                            "${ASKAP_ROOT}")
            # We now use shared libaries (June 2013) so do not want '.'
            # (really should never have "." set).
            # In the generated *.sh files which are for developer testing only,
            # want to use the built library (in package directory) and not the
            # installed one in pkgdir/install/lib.
            # For all the dependent packages use their installed version.
            ldpath = ldpath.replace('.:', '')
            ldpath = ldpath.replace('/install/lib', '', 1)
            binpath = appenv["ASKAP_BIN_PATH"].replace(env["ASKAP_ROOT"],
                                                        "${ASKAP_ROOT}")
            for f in appfiles:
                app = appenv.Program(f)
                self._apps.append(app[0])
                if len(self._sources) > 0: # i.e. the pkg library should exist
                    appenv.PrependUnique(LIBS=[self._name])
                appenv.PrependUnique(LIBPATH=["#"])
                runtxt = """#!/bin/sh
#
# ASKAP auto-generated file
#

ASKAP_ROOT=%(AR)s
export ASKAP_ROOT

PATH=%(PATH)s:${PATH}
export PATH

if [ "${%(LDPV)s}" !=  "" ]
then
    %(LDPV)s=%(LDPATH)s:${%(LDPV)s}
else
    %(LDPV)s=%(LDPATH)s
fi
export %(LDPV)s
""" % {'AR': env["ASKAP_ROOT"], 'LDPV': self.ldpathvar, 'LDPATH': ldpath,
       'PATH' : binpath}
                appenv["RUNTXT"] = runtxt
                createrun(app, appenv)
                appenv.Alias("apps", app)
            return True

        def AddTests(self):
            testenv = env.Clone()
            # Don't know why I need to re-import this
            # Otherwise the global env gets modified
            testenv.Tool("askap_package")

            # Add in $ASKAP_ROOT hierachy for test environment only.
            # This provides cppunit and testutils.
            testenv.AppendUnique(CPPPATH=["%s/include" % env['ASKAP_ROOT']])
            testenv.AppendUnique(LIBPATH=["%s/lib" % env['ASKAP_ROOT']])
            testenv.AppendUnique(LIBS=["testutils", "cppunit"])

            if len(self._sources) > 0: # i.e. the pkg library should exist
                testenv.PrependUnique(LIBS=[self._name])
            testenv.PrependUnique(LIBPATH=["#"])
            for package in self._subpackages:
                lenv = testenv.Clone()
                lenv.Prepend(CPPPATH=["tests/%s" % package])
                tfile = "tests/%s/t%s.cc" % (package, package)

                if os.path.exists(tfile):
                    self._tprogs.append(lenv.Program(source=tfile)[-1])
            testenv.Alias("buildtests", self._tprogs)
            expstr = "@echo export %s=$ASKAP_LIBRARY_PATH" % (self.ldpathvar)
            if os.environ.has_key(self.ldpathvar):
                expstr += ":%s" % os.environ[self.ldpathvar]
            self.rt = testenv.Command('runtests.sh', self._tprogs,
                                      ["@echo '#!/bin/sh' > $TARGET",
                                       expstr + ">> $TARGET",
                                       """@echo 'for prog in \'$SOURCES\';do $$prog;done' >> $TARGET""",
                                       "@sh $TARGET", "@rm -f $TARGET"])

        def RunTests(self):
            env.AlwaysBuild(self.rt)
            env.Alias("test", self.rt)

        def __call__(self):
            self.Build()
            self.BuildDoc()
            self.AddTests()
            hasapps = self.AddApps()
            self.RunTests()
            self.FuncTests()
            self.Install()
            deftargets = []
            if self.build_static:
                deftargets += ["static"]
            if self.build_shared:
                deftargets += ["shared"]
            if hasapps:
                deftargets.append("apps")
            env.Default(deftargets)

        def Build(self):
            if not len(self._sources):
                print "No sources for library creation specified."
                if len(self._subpackages) == 0:
                    print "Run AddSubPackage() to specify directories for library sources."
                else:
                    print "There are no source files in the specified SubPackage directories."
                return
            # for auto-generated files we need to remove the duplicates
            # after the first call to scons...
            # Note, this requires self._sources to be a list of strings not Nodes
            remove_duplicates(self._sources)
            if self.build_static:
                statlib = env.StaticLibrary(self._name, self._sources)
                self._libs.append(statlib[0])
                env.Alias("static", statlib[0])
            if self.build_shared:
                dylib = env.SharedLibrary(self._name, self._sources)
                self._libs.append(dylib[0])
                env.Alias("shared", dylib)

        def BuildDoc(self, doxyfile="doxy.conf"):
            docs = env.Doxygen(doxyfile)
            env.Alias("doc", docs)

        def Install(self, libdir="install/lib", incdir="install/include",
                bindir="install/bin"):
            if not os.path.exists(libdir):
                Mkdir(libdir)
            for lib in self._libs:
                instl = env.Install(libdir, lib)
                env.Alias("install", libdir)
            if not os.path.exists(libdir):
                Mkdir(libdir)
            for h in self._headers:
                dpath = os.path.dirname(h)
                insth = env.Install(os.path.join(incdir, dpath), h)
                env.Alias("install", os.path.join(incdir, dpath))
            if self._apps:
                if not os.path.exists(bindir):
                    Mkdir(bindir)
                for a in self._apps:
                    instb = env.Install(bindir, a)
                    # See #5450.  Do not need .sh version installed.
                    #instsh = env.Install(bindir, str(a)+".sh")
                    env.Alias("install", bindir)

        def _CreateConfig(self):
            conf = env
            fname = "%s.h" %  self._name
            cfgfile = file(fname, "w")
            name = self._name.replace("askap_", "")
            cfgtxt = """\
// This is an automatically generated file. Please DO NOT edit!
/// @file
///
/// Package config file. ONLY include in ".cc" files never in header files!

// std include
#include <string>

#ifndef %s_H
#define %s_H

  /// The name of the package
#define ASKAP_PACKAGE_NAME "%s"

/// askap namespace
namespace askap {
  /// @return version of the package
  std::string getAskapPackageVersion_%s();
}

  /// The version of the package
#define ASKAP_PACKAGE_VERSION askap::getAskapPackageVersion_%s()

#endif
""" % (self._name.upper(), self._name.upper(), name, name, name)
            try: 
               cfgfile.write(cfgtxt)
            finally:
               cfgfile.close()
            self._headers += [fname]
            env["PACKAGEINC"] = fname
            env.Clean("tidy", fname)
            # now make a cc file
            fname = "%s.cc" %  self._name
            cfgfile = file(fname, "w")
            cfgtxt = """\
// This is an automatically generated file. Please DO NOT edit!
/// @file
///
/// Package config file.

#include "%s.h"

/// askap namespace
namespace askap {

  /// @return version of the package
  std::string getAskapPackageVersion_%s() {
      return std::string("%s");
  }

}

""" % (self._name, name, utils.get_release_version())
            try: 
               cfgfile.write(cfgtxt)
            finally:
               cfgfile.close()
            self._sources += [fname]


    env.AskapPackage = AskapPackage

    def remove_duplicates(x):
        # find unique elements
        uk = dict([ (val, 1) for val in x]).keys()
        for k in uk:
            # remove all but last duplicate entry
            while x.count(k) > 1:
                del x[x.index(k)]

    # Add a 3rdParty library or ASKAP package to the environment
    # This will add the package path in ASKAP_LOCAL_ROOT (XXX what is this)
    # and/or ASKAP_ROOT.
    # @param pkgname The name of the package as in the repository, e.g.
    # lapack. Default None means that this is defined in local
    # dependencies.xyz
    # @param tag The location of the package, e.g.
    # 3rdParty/tags/lapack-3.1.1/lapack-3.1.1
    # @param libs The name of the libraries to link against,
    # default None is the same as the pkgname
    # @param libdir The location of the library dir relative to the package,
    # default None which will use settings in the package.info file
    # @param incdir The location of the include dir relative to the package,
    # default None which will use settings in the package.info file
    def AddAskapPackage(pkgname=None, tag=None,
                        libs=None, libdir=None, incdir=None):
        dep = Dependency()
        if env.has_key("update") and env["update"]:
            dep.selfupdate = True
        dep.add_package(pkgname, tag, libs, libdir, incdir)
        env.AppendUnique(CPPPATH=dep.get_includedirs())
        localld = os.path.join(os.path.abspath(os.curdir), "install", "lib")
        env["ASKAP_LIBRARY_PATH"] += \
                os.path.pathsep.join(('', localld, dep.get_ld_library_path()))
        ldvar = sys.platform == "darwin" and "DYLD_LIBRARY_PATH" or \
            "LD_LIBRARY_PATH"
        env["ENV"][ldvar] = env["ASKAP_LIBRARY_PATH"]
        for k,v in dep.get_env().iteritems():
            # make platform specific
            if k.find("LD_LIBRARY_PATH") > -1:
                k = ldvar
            env.PrependENVPath(k, v)
        env.PrependENVPath("PATH", dep.get_path())
        env["ASKAP_BIN_PATH"] = dep.get_path()
        env.AppendUnique(LIBPATH=dep.get_librarydirs())
        env.Append(LIBS=dep.get_libs())
        try:
            ice = dep.get_install_path("ice")
            env["ICE_HOME"] = ice
            env["INTERFACE_HOME"] = \
                os.path.join(dep.ASKAPROOT, dep.get_dep_path("interfaces"))
        except KeyError:
            pass
        env.AppendUnique(CPPFLAGS=dep.get_cppflags())
        env.Append(DOXYTAGS=dep.get_tagfiles())

    env.AddAskapPackage  = AddAskapPackage


    env["ASKAP_ROOT"] = os.environ.get("ASKAP_ROOT")
    if not env.get("ASKAP_LIBRARY_PATH"): env["ASKAP_LIBRARY_PATH"] = "."
    if not env.get("ASKAP_BIN_PATH"): env["ASKAP_BIN_PATH"] = "."

def exists(env):
    return True
