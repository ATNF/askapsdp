import os.path

from askapdev.rbuild.builders import Scons as Builder

b = Builder(".")

# Openmpi is virtual package, and want to know if the symlink has been created.
openmpi = b.dep.get_install_path("openmpi")
if not os.path.exists(openmpi):
    print "warn: Cannot build as no MPI support found on this platform."
else:
    b.build()
