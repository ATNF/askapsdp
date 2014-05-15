import os

from askapdev.rbuild.builders import Autotools as Builder
import askapdev.rbuild.utils as utils

platform = utils.get_platform()

builder = Builder()
builder.remote_archive = "mpe2-mpich2-1.5.tar.gz"

# Use MPI compiler wrapper (except on Cray where cc and c++ are wrappers)
if not os.environ.has_key("CRAYOS_VERSION"):
    openmpi = builder.dep.get_install_path("openmpi")
    builder.add_option("MPI_CC=%s/bin/mpicc" % openmpi)
    builder.add_option("MPI_F77=%s/bin/mpif77" % openmpi)

# MacOSX MPI is not necessarily built with f77 support,
# and on Linux we use gfortran
if platform['system'] == 'Darwin':
    builder.add_option("--disable-f77")
    java_home = os.getenv('JAVA_HOME')
    if java_home:
        builder.add_option("--with-java=%s" % java_home)
elif os.environ.has_key("CRAYOS_VERSION"):
    builder.add_option("F77=ftn")
else:
    builder.add_option("F77=gfortran")

builder.add_option("--disable-checkMPI")
builder.add_option("--disable-graphics")
builder.add_option("--disable-wrappers")
builder.add_option("--disable-collchk")

builder.nowarnings = True

builder.build()
