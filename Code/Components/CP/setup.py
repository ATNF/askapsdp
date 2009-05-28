from askapdev.rbuild import build
import os

def which(program):
    def is_exe(fpath):
        return os.path.exists(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        if os.environ.has_key("PATH"):
           for path in os.environ["PATH"].split(os.pathsep):
               exe_file = os.path.join(path, program)
               if is_exe(exe_file):
                   return exe_file

    return None


nobuild = {'benchmarks'  : "Not integrated into build system yet."}

for pkg, msg in nobuild.iteritems():
    print("warn: %s no install. %s" % (pkg, msg))

tobuild = ['mwcommon/trunk/build.py', 'askapparallel/trunk/build.py']

# Only build the MPI dependant components if mpicxx is present
if which("mpicxx"):
    tobuild.append('imager/trunk/build.py')
else:
    print "warn: No MPI support found on this platform."

# Build
build(tobuild)
