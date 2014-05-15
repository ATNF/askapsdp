import os
from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "mcpp-2.7.2.tar.gz"

if os.uname()[4] == 'x86_64':
    builder.add_option('--with-pic')
builder.add_option('--enable-mcpplib')
builder.add_option('--disable-shared')
builder.nowarnings = True

builder.build()
