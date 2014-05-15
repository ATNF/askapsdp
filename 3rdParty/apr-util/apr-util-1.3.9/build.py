from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "apr-util-1.3.9.tar.gz"

apr = builder.dep.get_install_path("apr")
expat = builder.dep.get_install_path("expat")
builder.add_option("--with-apr=%s" % apr)
builder.add_option("--with-expat=%s" % expat)
builder.add_option("--without-sqlite3")
builder.add_option("--without-sqlite2")
builder.add_option("--without-mysql")
builder.nowarnings = True

builder.build()
