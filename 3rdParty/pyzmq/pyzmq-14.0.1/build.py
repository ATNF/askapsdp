from askapdev.rbuild.builders import Setuptools as _Builder
import askapdev.rbuild.utils as utils

# pyzmq used non-standard configure step...
class Builder(_Builder):
    def _configure(self):
        zmq = self.dep.get_install_path("zeromq")
        cmd = "%s setup.py configure --zmq=%s" % (self._pycmd, zmq)
        utils.run("%s" % cmd, self.nowarnings)

builder = Builder()
builder.remote_archive = "pyzmq-14.0.1.tar.gz"

builder.build()
