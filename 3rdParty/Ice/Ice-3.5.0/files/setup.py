from setuptools import setup
from setuptools import find_packages

__author__ = "Malte.Marquarding@csiro.au"

setup (
    name         = "IcePy",
    description  = "Egg wrapper for Ice python bindings",
    author       = "Malte Marquarding",
    author_email = __author__,
    version      = "3.5.0",
    packages     = {'': ['python'],
                    'IceStorm': ['python.IceStorm'] , 
                    'IceGrid': ['python.IceGrid'] ,
                    'IceBox': ['python.IceBox'] ,
                    'IcePatch2': ['python.IcePatch2'] },
    package_dir  = {'': 'python'},
    package_data = { '': ["*.so*"] },
    zip_safe     = False
)
