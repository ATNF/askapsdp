__author__ = "malte.marquarding@csiro.au"

from setuptools import setup

setup (
    name         = "askapdev.helpers",
    description  = "various askap admin helperscripts",
    author       = "Tony Maher",
    author_email = "Tony.maher@csiro.au",
    scripts      = [ "scripts/pin_release_externals.sh",
                     "scripts/redminecsv2rst.py",
                     ],
    zip_safe     = False,
)
