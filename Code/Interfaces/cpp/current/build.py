from askapdev.rbuild.builders import Scons as Builder

b = Builder(".")
b.build()
# See Issue #2462 as to why we need to run the builder twice.
# The method .build() in 3rdParty and Tools does a clean before each build/install.
# But in Code the default is not to do a clean.  However for safety, since this is a double
# builder explicitly set the do_clean flag to false.
b.do_clean=False
b.build()
