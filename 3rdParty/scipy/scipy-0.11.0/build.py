import os.path

from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "scipy-0.11.0.tar.gz"

blas      = builder.dep.get_install_path("blas")
lapack    = builder.dep.get_install_path("lapack")
blaslib   = os.path.join(blas,   'lib/libblas.a')
lapacklib = os.path.join(lapack, 'lib/liblapack.a')

builder.add_env("BLAS", blaslib)
builder.add_env("LAPACK", lapacklib)
builder.nowarnings = True

builder.build()
