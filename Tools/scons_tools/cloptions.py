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
from  SCons.Variables import Variables
from SCons.Script import AddOption, GetOption

def generate(env):

    class CLOptions(object):
        def __init__(self):
            self.opts = {}
            self.variables = []

        def add_option(self, *args, **kw):
            AddOption(*args, **kw)
            key = kw.get('dest')
            value = GetOption(key)
            defvalue = kw.get('default')
            self.variables.append((key, '', defvalue))
            if value != defvalue:
                self.opts[key] = value

        def update(self, fname):
            if os.path.exists(fname) and not GetOption("silent") and not env.GetOption("help"):
                print "Restoring previous command-line options from '%s'" % fname
            vars = Variables(fname, self.opts)
            vars.AddVariables(*self.variables)
            vars.Update(env)
            vars.Save(fname, env)

        def add_pkg_option(self, libid, root=None, lib=None, libdir=None, 
                      incdir=None, help=None):
            libname = lib or libid
            self.add_str_option(libid+"-lib", libname,
                           help="%s library name (default: %s)" % (libname, libname))
            self.add_str_option(libid+"-root", root,
                           help="%s package root" % libid)
            self.add_str_option(libid+"-incdir", incdir,
                           help="%s package 'include' directory (overwrites '-root')" % libid)
            self.add_str_option(libid+"-libdir", libdir,
                           help="%s package 'lib' directory (overwrites '-root')" \
                               % libid)

        def add_str_option(self, optname, default=None, help=None):
            envopt = optname.replace("-", "_")
            self.add_option("--%s" % optname, dest=envopt, type="string",
                            default=default, help=help)

        def add_comp_option(self, optname, default=None, help=None):
            self.add_option("--with-%s" % optname.lower(), dest=optname,
                            type="string", default=default, help=help)



    env.CLOptions = CLOptions()

    def AddCommandLineOptions( ):
        """ Adds the build environment options to the opts.  """

        env.CLOptions.add_option("--enable-static", dest="enable_static",
                                 action="store_true", default=False,
                                 help="Enable building static libraries")
        env.CLOptions.add_option("--disable-shared", dest="disable_shared",
                                 action="store_true", default=False,
                                 help="Disable building dynamic (shared) libraries")
        env.CLOptions.add_option("--enable-hdf5", dest="enable_hdf5",
                                 action="store_true", default=False,
                                 help="Enable the HDF5 library")
        env.CLOptions.add_option("--enable-fftw3", dest="enable_fftw3",
                                 action="store_true", default=False,
                                 help="Enable the FFTW3 library")
        env.CLOptions.add_option("--disable-fftw3-threads", dest="disable_fftw3_threads",
                                 action="store_true", default=False,
                                 help="Disable use of threads in the FFTW3 library")
        env.CLOptions.add_option("--disable-dl", dest="disable_dl",
                                 action="store_true", default=False,
                                 help="Disable the use of dlopen")
        env.CLOptions.add_option("--enable-readline", dest="enable_readline",
                                 action="store_true", default=False,
                                 help="Enable the readline library")
        env.CLOptions.add_option("--data-dir", dest="data_dir", default=None,
                                 action="store", type="string",
                                 help="The location of the measures data directory to compile in as the default search location")
        env.CLOptions.add_option("--build-type", dest="build_type", default="opt",
                                 action="store", type="string",
                                 help="Build optimized 'opt' (default) or debug 'dbg'")

        env.CLOptions.add_pkg_option("hdf5")
        env.CLOptions.add_pkg_option("fftw3")
        env.CLOptions.add_pkg_option("fftw3-threads")
        env.CLOptions.add_pkg_option("dl")
        env.CLOptions.add_pkg_option("readline")
        env.CLOptions.add_pkg_option("blas")
        env.CLOptions.add_pkg_option("lapack")
        env.CLOptions.add_pkg_option("f2c", lib="gfortran")
        env.CLOptions.add_pkg_option("cfitsio")
        env.CLOptions.add_pkg_option("wcs")
        
        options = [("extra-cppflags", None, "Extra pre-processor flags"),
                   ("extra-cxxflags", None, "Extra c++ compiler falgs"),
                   ("extra-cflags", None, "Extra c compiler flags"),
                   ("extra-linkflags", None, "Extra linker flags"),
                   ("extra-fflags", None, "Extra fortran compiler flags"),
                   ("extra-includedir", None, "Extra 'include' dir(s)"),
                   ("extra-librarydir", None, "Extra 'lib' dir(s)"),
                   ("extra-ldlibrarypath", None, "Extra (DY)LD_LIBRARY_PATH"),
                   ("extra-libs", None, "Extra libraries for linker"),
                   ("extra-path", None, "Extra PATH (bin) to search"),
                   ("extra-root", None, "Extra hierachy root to search")]
        for opt in options:
            env.CLOptions.add_str_option(*opt)
        options = [("CC", None, "The c compiler"),
                   ("CXX", None, "The c++ compiler"),
                   ("FORTRAN", None, "The fortran compiler"),]
#                   ("LD", None, "The linker")]
        for opt in options:
            env.CLOptions.add_comp_option(*opt)

        PREFIX = "prefix"
        EPREFIX = "eprefix"
        BINDIR = "bindir"
        LIBDIR = "libdir"
        INCLUDEDIR = "includedir"
        SHAREDIR = "sharedir"
        
        defdir = "/usr/local"
    
        env.CLOptions.add_option("--"+PREFIX, dest=PREFIX,
                  type="string", default=defdir, 
                  help="The installation prefix (default: %s)" % defdir)
        env.CLOptions.add_option("--"+EPREFIX, dest=EPREFIX,
                  type="string", default=defdir)
        env.CLOptions.add_option("--"+BINDIR, dest=BINDIR,
                  type="string", default=None, 
                  help="The installation bin directory (default: %s/bin)" % defdir)
        env.CLOptions.add_option("--"+LIBDIR, dest=LIBDIR,
                  type="string", default=None, 
                  help="The installation lib directory (default: %s/lib)" % defdir)
        env.CLOptions.add_option("--"+INCLUDEDIR, dest=INCLUDEDIR,
                  type="string", default=None, 
                  help="The installation include directory (default: %s/include)" % defdir)
        env.CLOptions.add_option("--"+SHAREDIR, dest=SHAREDIR,
                  type="string", default=None, 
                  help="The installation share directory (default: %s/share)" % defdir)
        
        env.CLOptions.update('options.cache')

    AddCommandLineOptions()

def exists(env):
    return 1
