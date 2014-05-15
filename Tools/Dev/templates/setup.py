__author__ = "Tony.Maher@csiro.au"

from askapdev.rbuild.setup import setup

setup (
    name         = "askapdev.templates",
    description  = "scripts and templates for creating package skeletons",
    author       = "Tony Maher",
    author_email = "Tony.Maher@csiro.au",
    packages     = ["askapdev",
                    "askapdev.templates",
                    "templates",
                   ],
    package_data = {"templates": ["cpp/*.tmpl", "java/*.tmpl",
                                  "python/*/*.tmpl", "epics/*.tmpl"]},
    scripts      = ["scripts/create_cpkg", "scripts/create_javapkg",
                    "scripts/create_pypkg", "scripts/create_epicspkg"],
    zip_safe     = False,
    namespace_packages = ['askapdev'],
)
