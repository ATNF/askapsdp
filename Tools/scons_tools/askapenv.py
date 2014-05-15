import os
import sys

from SCons.Environment import Environment
from SCons.Variables import Variables, BoolVariable
from SCons.Script import ARGUMENTS

# This is always needed as it defines the ASKAP scons environment
askaptoolpath = os.path.join(os.environ['ASKAP_ROOT'], 'share', 'scons_tools')

# The build environment.

env = Environment(ENV =  { 'PATH' : os.environ[ 'PATH' ],
                           'HOME' : os.environ[ 'HOME' ] },
                  toolpath = [askaptoolpath],
                  tools = ['default', 'askap_package', 'doxybuilder',
                           'functestbuilder', 'icebuilder', 'cloptions',
                           'casa',
                           ]
                  )

# Importing TERM allows programs (such as clang) to produce colour output
# if the terminal supports it
if 'TERM' in os.environ:
    env['ENV']['TERM'] = os.environ['TERM']

opts = Variables('sconsopts.cfg', ARGUMENTS)
opts.Add(BoolVariable("mpich", "Use MPI", False))
opts.Add(BoolVariable("mpi", "Use MPI", False))
opts.Add(BoolVariable("openmp", "Use OpenMP", False))
opts.Add(BoolVariable("update", "svn update?", False))
opts.Update(env)

env.AppendUnique(CCFLAGS=['-Wall'])

# Debian etch 32 bit has two threading packages.
# No easy way to determine a Debian system, but the combination
# of the following tests should be sufficent.
# This code is duplicated in rbuild builder.py to handle 3rdParty packages.
# Also require additional symlink
# cd /usr/lib/nptl && ln -s librt.so.1 librt.so
# Commented out for now due to ticket:733
#if sys.platform == 'linux2' and os.uname()[4] == 'i686':
#    if os.path.exists('/usr/lib/nptl'):
#        env.Append(CFLAGS=['-I/usr/include/nptl'])
#        env.Append(CPPFLAGS=['-I/usr/include/nptl'])
#        env.Append(LINKFLAGS=['-L/usr/lib/nptl'])

if env['mpi'] or env['mpich']:
    if not env.Detect("mpicc"):
        print "mpi not found"
        env.Exit(1)
    env["CC"] = "mpicc"
    env["CXX"] = "mpicxx"
    env["LINK"] = "mpicxx"
    env["SHLINK"] = "mpicxx"
    env.AppendUnique(CPPFLAGS=['-DHAVE_MPI'])

if env['openmp']:
    env.AppendUnique(CCFLAGS=['-fopenmp'])
    env.AppendUnique(LINKFLAGS=['-fopenmp'])

# Overwrite for Cray
if os.environ.has_key("CRAYOS_VERSION"):
    env["ENV"] = os.environ
    env["CC"] = "cc"
    env["CXX"] = "CC"
    env["LINK"] = "CC"
    env["SHLINK"] = "CC"
    env.AppendUnique(LINKFLAGS=['-dynamic'])
    env.AppendUnique(CPPFLAGS=['-DHAVE_MPI'])

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
#if os.path.exists("functests"):
#    env.SConscript("functests/SConscript", exports='env')
