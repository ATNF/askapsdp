# @file
import os
import sys
import shutil
import md5

from recursivebuild import run
from recursivebuild.utils import q_print

def create_sig():
    md5sig = md5.new(file("build.py").read())
    outsig = file(".packagesig", "w")
    outsig.write(md5sig.hexdigest())
    outsig.close()

def cleandata():
    run("/bin/rm -f data/*uJy_*")

def clean(files):
    for f in files:
        if os.path.exists(f):
            if os.path.isdir(f):
                shutil.rmtree(f)
            else:
                os.remove(f)

def install(pkg):
    print("running install script")
    run("sh createSourceLists.sh")
    create_sig()

#
# main
#

bfile   = 'build.py'
sigfile = '.packagesig'
pkg     = 'simData'

print("warn: non-standard build.py script used for %s" % pkg)

if "clean" in sys.argv[1:]:
    clean([sigfile])
    cleandata()
    sys.exit(0)

if "install" in sys.argv[1:]:
    if os.path.exists(sigfile):
        pkgsig = file(sigfile).read()
        md5sig = md5.new(file(bfile).read()).hexdigest()
        if pkgsig == md5sig:
            sys.exit(0)
        else:
            q_print("info: %s checksum differs from value in %s - install." %
                    (bfile, sigfile))
    else:
        q_print("info: No %s file - install" % sigfile)
    install(pkg)
else:
    print("warn: unknown build option in %s" % sys.argv[1:])
