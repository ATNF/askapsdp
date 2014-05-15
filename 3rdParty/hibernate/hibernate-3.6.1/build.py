from askapdev.rbuild.builders import Data as Builder

builder = Builder(pkgname="hibernate-distribution-3.6.1.Final")
builder.remote_archive = "hibernate-distribution-3.6.1.Final.tar.gz"
builder.build()
