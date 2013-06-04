from askapdev.rbuild.builders import Scons as Builder

b = Builder(".")
b.build()

# See Issue #2462 as to why we need to run the builder twice.
# build() method in Code removes the install directory so need
# to set the do_clean flag to False - See Issue #3307.
b.do_clean=False
b.build()
