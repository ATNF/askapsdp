from askapdev.rbuild.builders import Autotools as Builder

builder = Builder("cfitsio", buildtargets=['shared'])
builder.remote_archive = "cfitsio3350.tar.gz"
builder.nowarnings = True

builder.build()
