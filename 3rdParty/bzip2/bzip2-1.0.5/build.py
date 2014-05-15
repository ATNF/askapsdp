import os.path

from askapdev.rbuild.builders import Autotools as Builder

prefix = os.path.join(os.path.abspath(os.path.curdir), "install")

builder = Builder(confcommand=None,
                  installcommand='make install PREFIX=%s' % prefix)
builder.remote_archive = "bzip2-1.0.5.tar.gz"
builder.nowarnings = True
builder.build()
