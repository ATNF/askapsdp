import os
import optparse
import shutil
import subprocess
import sys

## execute an svn up command
#
#  @param thePath The path to update
#  @param recursive Do a recursive update
def update_command(thePath, recursive=False):
    ropt = "-N"
    if recursive:
        ropt = ""
    comm = "svn up %s %s" % (ropt, thePath)
    p = subprocess.Popen(comm, stdout=subprocess.PIPE, 
                         stderr=subprocess.PIPE, shell=True, close_fds=True)
    c = p.communicate()
    if "-q" not in sys.argv:
        print c[0]

##  svn update a path in the repository. This will be checked out as necessary
#
#   @param thePath The repository path to update. 
def update_tree(thePath):
    if os.path.exists(thePath):
        update_command(thePath, recursive=True)
        return
    pathelems = thePath.split(os.path.sep)
    # get the directory to check out recursively
    pkgdir = pathelems.pop(-1)
    pathvisited = ""
    for pth in pathelems:
        pathvisited += pth + os.path.sep
        if not os.path.exists(pathvisited):
            update_command(pathvisited)
    pathvisited += pkgdir
    update_command(pathvisited, recursive=True)

## remove files or directories
#
# @param paths The list of paths to remove.
def remove_paths(paths):
    for p in paths:
        if os.path.exists(p) and p != ".":
            print ">>> Attempting to remove %s" % p
            if os.path.isdir(p):
                shutil.rmtree(p)
            else:
                os.remove(p)

##
# main
#
usage = "usage: python bootstrap.py [options]"
desc  = "Bootstrap the ASKAP build environment"
parser = optparse.OptionParser(usage, description=desc)
parser.add_option('-n', '--no-update', dest='no_update',
                  action="store_true", default=False,
                  help='Do not use svn to checkout anything. Default is to update and checkout Tools.')

parser.add_option('-p', '--preserve', dest='preserve',
                  action="store_true", default=False,
                  help='Keep pre-existing bootstrap files (though they maybe overwritten). Default is to remove.')
recursivebuild_path = "Tools/Dev/recursivebuild"
virtualenv_path     = "Tools/Dev/virtualenv"

invoked_path  = sys.argv[0]
absolute_path = os.path.abspath(invoked_path)

os.chdir(os.path.dirname(absolute_path))

(opts, args) = parser.parse_args()

if opts.preserve:
    print ">>> No pre-clean up of existing bootstrap generated files."
else:
    print ">>> Attempting removal prexisting bootstrap files (if they exist)."
    remove_paths(["lib",  "bin", "include", "initaskap.sh", "initaskap.csh",
        ".Python"])

if opts.no_update:
    print ">>> No svn updates as requested but this requires that the"
    print ">>> Tools tree already exists."
else:
    print ">>> Updating Tools tree."
    update_tree("Tools")

if os.path.exists(virtualenv_path):
    print ">>> Attempting to create python virtual environment."
    os.system("cd %s && python bootstrap.py" % virtualenv_path)
else:
    print ">>> %s does not exist." % os.path.abspath(virtualenv_path)
    sys.exit()

print ">>> Attempting to create initaskap.sh."
os.system("python initenv.py >/dev/null")
print ">>> Attempting to create initaskap.csh."
os.system("python initenv.py -s tcsh >/dev/null")

if os.path.exists(recursivebuild_path):
    print ">>> Attempting to build recursivebuild."
    os.system(". initaskap.sh && cd %s && python setup.py -q install"
                % recursivebuild_path)
else:
    print ">>> %s does not exist." % os.path.abspath(recursivebuild_path)
    sys.exit()

if not opts.preserve:
    print ">>> Attempting to clean all the Tools."
    os.system(". initaskap.sh && cd Tools && python setup.py -q clean > /dev/null 2>& 1")
print ">>> Attempting to build all the Tools."
os.system(". initaskap.sh && cd Tools && python setup.py -q install")
