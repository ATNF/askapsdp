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

from SCons.Environment import Environment
from SCons.Variables import Variables, BoolVariable
from SCons.Script import ARGUMENTS

# Returns true if the environment has "modules" support
def has_environment_modules():
    return os.environ.has_key("MODULESHOME")

# Does the platform have MPI capability that can be used explicitly
# by using the mpicc * mpicxx compiler wrappers?
def has_explicit_mpi(env):
    return env.Detect("mpicc") and env.Detect("mpicxx")

# Some platforms have MPI support which must be explicitly setup.
# That is the mpicc/mpicxx compiler wrappers need to be used explicitly
# Others, such as the Cray environment have MPI support already wrapped
# in the CC & CXX commands
def has_implicit_mpi():
    return os.environ.has_key("CRAYOS_VERSION")

# This is always needed as it defines the ASKAP scons environment
askaptoolpath = os.path.join(os.environ['ASKAP_ROOT'], 'share', 'scons_tools')

# The build environment.
env = Environment(ENV =  { 'PATH' : os.environ[ 'PATH' ],
                           'HOME' : os.environ[ 'HOME' ] },
                  toolpath = [askaptoolpath],
                  tools = ['default', 'askap_package', 'doxybuilder',
                           'functestbuilder', 'icebuilder', 'cloptions' ]
                  )

# Importing TERM allows programs (such as clang) to produce colour output
# if the terminal supports it
if 'TERM' in os.environ:
    env['ENV']['TERM'] = os.environ['TERM']

opts = Variables('sconsopts.cfg', ARGUMENTS)
opts.Add(BoolVariable("nompi", "Disable MPI", False))
opts.Add(BoolVariable("openmp", "Use OpenMP", False))
opts.Add(BoolVariable("update", "svn update?", False))
opts.Update(env)

# Standard compiler flags
env.AppendUnique(CCFLAGS=['-Wall'])
env.AppendUnique(CCFLAGS=['-O2'])
env.AppendUnique(CCFLAGS=['-g'])
env.AppendUnique(CCFLAGS=['-DASKAP_DEBUG'])

# If the system has environment modules support we need to import
# the whole environment
if has_environment_modules():
    env["ENV"] = os.environ

# Support setting of "RBUILD_NOMPI" in the environment, because forgetting
# to pass nompi=1 on the command line is a common error
if os.environ.has_key('RBUILD_NOMPI') and os.environ['RBUILD_NOMPI'] == '1':
    print "info: RBUILD_NOMPI=1 found in env, building without MPI support"
    env['nompi'] = 1

# Setup MPI support
if has_implicit_mpi():
    if env['nompi']:
        print "error: Cannot disable MPI on this platform"
        env.Exit(1)
    env.AppendUnique(CPPFLAGS=['-DHAVE_MPI'])

if not env['nompi'] and not has_implicit_mpi():
    if has_explicit_mpi(env):
            env["CC"] = "mpicc"
            env["CXX"] = "mpicxx"
            env["LINK"] = "mpicxx"
            env["SHLINK"] = "mpicxx"
            env.AppendUnique(CPPFLAGS=['-DHAVE_MPI'])
    else:
        print "warn: No MPI support detected, compiling without"

# Setu OpenMP support
if env['openmp']:
    env.AppendUnique(CCFLAGS=['-fopenmp'])
    env.AppendUnique(LINKFLAGS=['-fopenmp'])

# Overwrite for Cray, need to use the standard compiler wrappers
# By default gcc/g++ are used
if os.environ.has_key("CRAYOS_VERSION"):
    env["ENV"] = os.environ
    env["CC"] = "cc"
    env["CXX"] = "CC"
    env["LINK"] = "CC"
    env["SHLINK"] = "CC"
    env.AppendUnique(LINKFLAGS=['-dynamic'])

# use global environment definitions
ASKAP_ROOT = os.getenv('ASKAP_ROOT')
envfiles =  ['%s/env.default' % ASKAP_ROOT,]
for e in envfiles:
    if os.path.exists(e):
        print("askapenv: processing environment file: %s" % e)
        opts = []
        for line in open(e, "r"):
            line = line.strip()
            if line and not line.startswith("#"):
                (k, v) = line.split('=')
                env[k] = v
