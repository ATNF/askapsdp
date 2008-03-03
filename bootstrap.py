import os
import sys
import subprocess

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

update_tree("Tools")
os.system("cd Tools/Dev/setuptools; python bootstrap.py")
os.system("python initenv.py >/dev/null")
os.system(". initaskap.sh; cd Tools/Dev/recursivebuild; python setup.py -q install")
os.system(". initaskap.sh; cd Tools; python setup.py -q install")
