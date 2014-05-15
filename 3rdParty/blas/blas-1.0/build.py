from askapdev.rbuild.builders import Scons as Builder

builder = Builder(pkgname="BLAS", archivename="blas")
builder.remote_archive = "blas.tgz"
builder.add_file("files/SConstruct")

builder.build()
