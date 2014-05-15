import os

from askapdev.rbuild.builders import Autotools as Builder

def callback():
    if os.path.exists("lib64") and not os.path.exists("lib"):
        os.symlink("lib64", "lib")

builder = Builder()
builder.remote_archive = "Blob-1.2.tar.gz"

common   = builder.dep.get_install_path("common")
boost    = builder.dep.get_install_path("boost")
casacore = builder.dep.get_install_path("casacore")
log4cxx  = builder.dep.get_install_path("log4cxx")

builder.add_option("--with-cppflags='-DUSE_NO_TH_ETHERNET'")
builder.add_option("--with-log4cxx=%s" % log4cxx)
builder.add_option("--with-log4cxx-libdir=%s/lib" % log4cxx)
builder.add_option("--enable-shared")
builder.add_option("--with-ldflags='-dynamic'")
builder.add_option("--without-log4cplus")
builder.add_option("--with-optimize='-O2 -fPIC'")
builder.add_option("--without-shmem")
builder.add_option("--without-insuretools")
builder.add_option("--without-puretools")
builder.add_option("--with-common=%s" % common)
builder.add_option("--with-common-libdir=%s/lib" % common)
builder.add_option("--with-boost=%s" % boost)
builder.add_option("--with-boost-libdir=%s/lib" % boost)
builder.add_option("--with-casacore=%s" % casacore)
builder.add_option("--with-casacore-libdir=%s/lib" % casacore)
builder.add_option("--without-wcs")
builder.add_postcallback(callback)
builder.nowarnings = True

builder.build()
