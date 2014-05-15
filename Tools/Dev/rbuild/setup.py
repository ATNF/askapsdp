__author__ = "robert.crida@ska.ac.za"

from askapdev.rbuild.setup import setup

setup (
    name         = "askapdev.rbuild",
    description  = "scripts for recursively building packages",
    author       = "Tony Maher",
    author_email = "Tony.maher@csiro.au",
    packages     = [ "askapdev",
                     "askapdev.rbuild",
                     "askapdev.rbuild.builders",
                     "askapdev.rbuild.dependencies",
                     "askapdev.rbuild.utils",
                     "askapdev.rbuild.setup",
                     "askapdev.rbuild.setup.commands",
                     "askapdev.rbuild.debian",
                     "templates",
                   ],
    package_data = { "templates": ["*"], 
                     "askapdev.rbuild.debian": 
                     ["data/[!.]*"] },
    scripts      = [ "scripts/rbuild",
                     "scripts/debianise.py",
                     "scripts/askap-debpackage",
                   ],
    zip_safe     = False,
    #
    namespace_packages = ['askapdev'],
)
