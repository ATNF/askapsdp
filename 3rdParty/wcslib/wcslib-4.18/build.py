from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "wcslib-4.18.tar.bz2"

builder.add_env("CFLAGS", "-fPIC")
builder.add_env("CPPFLAGS", "-fPIC")
builder.add_env("FFLAGS", "-fPIC")
builder.parallel = False
builder.nowarnings = True

cfitsio = builder.dep.get_install_path("cfitsio")
builder.add_option("--with-cfitsiolib=%s" % cfitsio+"/lib")
builder.add_option("--with-cfitsioinc=%s" % cfitsio+"/include")
builder.add_option("--without-pgplot")

builder.build()
