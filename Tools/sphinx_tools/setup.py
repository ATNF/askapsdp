__author__ = "Malte.Marquarding@csiro.au"

from askapdev.rbuild.setup import setup

setup (
    name         = "askapdev.sphinx",
    description  = "sphinx utilities",
    author       = "Malte Marquarding",
    author_email = __author__,
    packages     = [ "askapdev",
                     "askapdev.sphinx",
                   ],
    zip_safe     = True,
    namespace_packages = ['askapdev'],
)
