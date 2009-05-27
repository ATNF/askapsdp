from askapdev.rbuild import build
import os
import sys

STD_LIB_PATHS = ['/usr/lib', '/usr/local/lib']

def whichlib(program):
   if sys.platform == "darwin":
       lib_envvar = "DYLD_LIBRARY_PATH"
   else:
       lib_envvar = "LD_LIBRARY_PATH"

   def is_readable(fpath):
       return os.path.exists(fpath) and os.access(fpath, os.R_OK)

   fpath, fname = os.path.split(program)
   if fpath:
       if is_readable(program):
           return program
   else:
       if os.environ.has_key(lib_envvar):
           search_paths = os.environ[lib_envvar].split(os.pathsep)
           search_paths.extend(STD_LIB_PATHS)
       else:
           search_paths = STD_LIB_PATHS

       for path in search_paths:
           lib_file = os.path.join(path, program)
           if is_readable(lib_file):
               return lib_file

   return None

nobuild = {'benchmarks'  : "Not integrated into build system yet."}

for pkg, msg in nobuild.iteritems():
    print("warn: %s no install. %s" % (pkg, msg))

tobuild = ['mwcommon/trunk/build.py', 'askapparallel/trunk/build.py']

# Only build the MPI dependant components if libmpi.so is present
if sys.platform == "darwin":
    lib_name = "libmpi.dylib"
else:
    lib_name = "libmpi.so"

if whichlib(lib_name):
    tobuild.append('imager/trunk/build.py')
else:
    print "No MPI support found on this platform"

# Build
build(tobuild)
