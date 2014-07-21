from askapdev.rbuild.builders import Ant as Builder

builder = Builder(".")
builder.add_run_script("skymodelsvc.sh", "-Xmx1024m askap/cp/skymodelsvc/Server")
builder.add_run_script("caldatasvc.sh", "-Xmx1024m askap/cp/caldatasvc/Server")
builder.add_run_script("rfisourcesvc.sh", "-Xmx1024m askap/cp/rfisourcesvc/Server")

builder.build()
