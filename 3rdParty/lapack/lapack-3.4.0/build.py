import glob
import os
import shutil

from askapdev.rbuild.builders import Autotools as _Builder

class Builder(_Builder):
    def _install(self):
        libs = glob.glob("liblapack*")
        dstdir = os.path.join(self._prefix, 'lib')
        if not os.path.exists(dstdir):
            os.makedirs(dstdir)
        for lib in libs:
            shutil.copy(lib, dstdir)


builder = Builder(confcommand=None)
builder.remote_archive = "lapack-3.4.0.tgz"

blas = builder.dep.get_dep_path("blas")

builder.add_option('-fPIC')
builder.add_file("files/make.inc")
builder.replace("make.inc", "@@@blas@@@", blas)
builder.parallel = False

builder.build()
