import platform
from askapdev.rbuild.builders import Autotools as Builder

# Build twice, a single-precision version of the library plus a double
# precision version. The library names are different so will co-exist
# in the same install directory

def secondbuilder():
    builder2 = Builder()
    if platform.machine() == "x86_64":
        builder2.add_option('"CFLAGS=-fPIC -O2"')
    builder2.add_option('--enable-float --enable-threads')
    builder2.do_clean = False
    builder2.build()
    
builder = Builder()
builder.remote_archive = "fftw-3.3.3.tar.gz"

if platform.machine() == "x86_64":
    builder.add_option('CFLAGS="-fPIC -O2"')
    builder.add_option('--enable-sse2')

builder.add_option('--enable-threads')
builder.add_postcallback(secondbuilder)
 
builder.build()
