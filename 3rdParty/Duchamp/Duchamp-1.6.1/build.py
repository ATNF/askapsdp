from askapdev.rbuild.builders import Autotools as Builder

builder = Builder(buildtargets=['duchamp','lib'])
builder.remote_archive = "Duchamp-1.6.1.tar.gz"

cfitsio = builder.dep.get_install_path("cfitsio")
wcslib  = builder.dep.get_install_path("wcslib")

builder.add_option("--without-pgplot")
builder.add_option("--with-cfitsio=%s" % cfitsio)
builder.add_option("--with-wcslib=%s" % wcslib)

builder.build()
