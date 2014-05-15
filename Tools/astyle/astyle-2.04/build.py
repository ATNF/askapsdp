import os

from askapdev.rbuild.builders import Autotools as Builder
import askapdev.rbuild.utils as utils

ASKAP_ROOT = os.getenv('ASKAP_ROOT')
install_cmd = "make prefix=%s install" % ASKAP_ROOT

platform = utils.get_platform()

if platform['system'] == 'Darwin':
    archext = "macosx"
    buildext = "mac"
else:
    archext = "linux"
    buildext = "gcc"

if platform['system'] == 'FreeBSD':
    install_cmd = 'g' + install_cmd

builder = Builder(pkgname="astyle/build/%s" % buildext,
                  confcommand=None,
                  installcommand=install_cmd)
builder.remote_archive = "astyle_2.04_" + archext + ".tar.gz"

# It extracts to sub directory astyle not the expected astyle-2.04.
builder.add_extra_clean_targets('astyle')
builder.nowarnings = True
builder.build()
