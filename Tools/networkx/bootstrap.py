import os
import sys
import urllib2

nx = "networkx-1.5"
askap_root = os.environ.get("ASKAP_ROOT", os.path.abspath("../../"))
python_exe = os.path.join(askap_root, "bin", "python")

os.chdir(nx)
remote = os.environ.get("RBUILD_REMOTE_ARCHIVE", "")
pkg = "%s.tar.gz" % nx
if remote:
    pth = os.path.join(remote, pkg)
    url = urllib2.urlopen(pth)
    with open(pkg, "wb") as of:
        of.write(url.read())
    print "Downloading", remote

os.system("tar xvzf %s" % pkg)
os.chdir(nx)
os.system("%s setup.py -q install" % python_exe)
