from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "logilab.pylintinstaller-0.1.1.tar.gz"
builder.build()
