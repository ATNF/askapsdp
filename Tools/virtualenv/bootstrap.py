import shutil
import os
import sys
import urllib2

version = "1.11.4"
virtualenv = "virtualenv-%s" % version
python_ver = sys.version[:3]
python_exe = sys.executable
askap_root = os.environ.get("ASKAP_ROOT", os.path.abspath("../../"))
install_dir = "%s/install" % os.getcwd()

if os.path.isdir(install_dir):
    shutil.rmtree(install_dir)

os.chdir(virtualenv)
remote = os.environ.get("RBUILD_REMOTE_ARCHIVE", "")
pkg = "%s.tar.gz" % virtualenv
if remote:
    pth = os.path.join(remote, pkg)
    url = urllib2.urlopen(pth)
    with open(pkg, "wb") as of:
        of.write(url.read())
    print "Downloading", remote

os.system("tar xvzf %s" % pkg)
os.chdir("%s" % virtualenv)
os.system("%s virtualenv.py --no-site-packages %s" % (python_exe, askap_root))
# For use with release process.
os.system("%s virtualenv.py --no-site-packages %s" % (python_exe, install_dir))
