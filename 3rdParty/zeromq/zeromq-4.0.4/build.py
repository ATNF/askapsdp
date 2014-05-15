from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "zeromq-4.0.4.tar.gz"
builder.nowarnings = True
builder.add_install_file("files/zmq.hpp", "include")
builder.build()
