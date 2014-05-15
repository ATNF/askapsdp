from askapdev.rbuild.builders import Data as Builder

builder = Builder()
builder.remote_archive = "mysql-connector-java-5.1.15.tar.gz"
builder.build()
