from askapdev.rbuild.builders import Ant as Builder

builder = Builder(".")
builder.add_run_script("cpmanager.sh", "askap/cp/manager/CpManager")

builder.build()
