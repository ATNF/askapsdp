import os
import glob
import shutil
from askapdev.rbuild.builders import Builder as _Builder
import askapdev.rbuild.utils as utils

class Builder(_Builder):

    def _build(self):
        pass

    def _install(self):
        jars = glob.glob("*.jar")        
        dstdir = os.path.join(self._prefix, 'lib')
        if not os.path.exists(dstdir):
            os.makedirs(dstdir)
        for jar in jars:
            shutil.copy(jar, dstdir)

builder = Builder()
builder.remote_archive = "apache-log4j-1.2.15.tar.gz"
builder.build()
