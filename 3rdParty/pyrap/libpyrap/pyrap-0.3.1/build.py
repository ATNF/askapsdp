import subprocess
from askapdev.rbuild.builders import Builder as _B
from askapdev.rbuild.builders import Scons as Builder

#introspect numpy for include dir...
b = _B()
env = b._get_env()
p = subprocess.Popen("{0} python -c 'import numpy; print numpy.get_include()'".format(env),  stdout=subprocess.PIPE, shell=True)
numpy = p.communicate()[0].strip()

builder = Builder()
builder.remote_archive = "pyrap-0.3.1.tar.bz2"

casacore = builder.dep.get_install_path("casacore")
bpython = builder.dep.get_install_path("boost")

builder.add_option("--casacore-root=%s" % casacore)
builder.add_option("--numpy-incdir=%s" % numpy )
builder.add_option("--boost-root=%s" % bpython )
# scons builder uses prefix=, this package uses --prefix=
builder.add_option("--prefix=%s" % builder._prefix)
builder.nowarnings = True
builder.build()
