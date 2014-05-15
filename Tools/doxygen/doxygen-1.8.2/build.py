import shutil
import os

from askapdev.rbuild.builders import Autotools as Builder
import askapdev.rbuild.utils as utils

ASKAP_ROOT = os.path.abspath(os.getenv('ASKAP_ROOT'))
DOXYGEN_VER = '1.8.2'
DOXYGEN_DIR = os.path.join(ASKAP_ROOT, 'share', 'doxygen')
DOXYSYMLINK = os.path.join(DOXYGEN_DIR, 'doxygen.conf')

CFILES = ['tmake/lib/macosx-c++/tmake.conf',
          'tmake/lib/macosx-intel-c++/tmake.conf',
          'tmake/lib/macosx-uni-c++/tmake.conf']

platform = utils.get_platform()

def configure():
    utils.run("./configure --prefix %s" % ASKAP_ROOT)

def postinstall():
    if not os.path.exists(DOXYGEN_DIR):
        os.makedirs(DOXYGEN_DIR)
    shutil.copy('files/doxygen.css', DOXYGEN_DIR)
    shutil.copy('files/doxygen-%s.conf' % DOXYGEN_VER, DOXYGEN_DIR)
    if os.path.exists(DOXYSYMLINK):
        os.remove(DOXYSYMLINK)
    cwd = os.getcwd()
    os.chdir(DOXYGEN_DIR)
    os.symlink('doxygen-%s.conf' % DOXYGEN_VER, 'doxygen.conf')
    os.chdir(cwd)
    
builder = Builder()
builder.remote_archive = "doxygen-" + DOXYGEN_VER + ".src.tar.gz"

# overwrite _configure method as it isn't autotools
builder._configure = configure

# http://trac.macports.org/changeset/101062 - remove clang flag so can build
# with gcc.
if platform['system'] == 'Darwin':
    for cfile in CFILES:
        builder.replace(cfile, '-Wno-invalid-source-encoding', '')

builder.add_postcallback(postinstall)
#builder.parallel = False
builder.nowarnings = True

builder.build()
