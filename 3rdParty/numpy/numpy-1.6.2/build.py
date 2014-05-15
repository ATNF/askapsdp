import os.path
import glob
import shutil

from askapdev.rbuild.builders import Setuptools as Builder

# hack as install_header setup target doesn't seem to work
# numpy-1.6.1/build/src.linux-x86_64-2.6/numpy/core/include/
def callback():
    bdistdir = glob.glob('%s/build/src*' % builder._package)
    if os.path.exists(bdistdir[0]):
        incdir = os.path.join(bdistdir[0], 'numpy', 'core', 'include')
        if not os.path.exists('install/include'):
            shutil.copytree(incdir ,'install/include')
    
builder = Builder()
builder.remote_archive = "numpy-1.6.2.tar.gz"

blas      = builder.dep.get_install_path("blas")
lapack    = builder.dep.get_install_path("lapack")
blaslib   = os.path.join(blas,   'lib', 'libblas.a')
lapacklib = os.path.join(lapack, 'lib', 'liblapack.a')

builder.add_env("BLAS", blaslib)
builder.add_env("LAPACK", lapacklib)
builder.add_postcallback(callback)
builder.nowarnings = True

builder.build()
