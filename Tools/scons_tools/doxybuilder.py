import os
import string

import SCons.Action
import SCons.Defaults
import SCons.Builder
from askapdev.rbuild.utils import tag_name

def doxy_emitter(target, source, env):
    insrc =  str(source[0])
    outsrc = insrc+".tmp"
    curd = os.path.abspath(".")
    doxytagname = tag_name(curd)
    newsrc = open(outsrc, "w")
    cpr = env["ASKAP_ROOT"]
    newsrc.write(
"""@INCLUDE_PATH = %s
@INCLUDE = doxygen.conf
HTML_STYLESHEET = %s
OPTIMIZE_OUTPUT_JAVA = NO
EXTRACT_STATIC = YES
GENERATE_TAGFILE = %s
FILE_PATTERNS = *.h *.tcc *.cc *.c
""" % (os.path.join(cpr, env["DOXYINC"]),
       os.path.join(cpr, env["DOXYCSS"]),
       doxytagname,
       )
)
    newsrc.write(file(insrc, "r").read())
    if env.has_key("DOXYTAGS"):
        if hasattr(env["DOXYTAGS"], "__len__") and len(env["DOXYTAGS"]):
            newsrc.write("TAGFILES = %s\n" % string.join(env["DOXYTAGS"]))
    if env.has_key("PACKAGEINC") and env["PACKAGEINC"] is not None:
        newsrc.write("INPUT += %s\n" % env["PACKAGEINC"])

    newsrc.close()
    env["DOXYGEN"] = os.path.join(cpr, env["DOXYGEN"])
    return target, [outsrc]

def doxy_str(target, source, env):
    return "Generating doxygen documentation"


doxy_action = SCons.Action.Action("$DOXYGEN $SOURCES", doxy_str)

doxygen_builder = SCons.Builder.Builder(
    action = [doxy_action, SCons.Defaults.Delete("$SOURCES")],
    src_suffix =  ".conf",
    emitter = doxy_emitter
    )


def generate(env):
    env.AppendUnique(
        DOXYGEN = 'bin/doxygen',
        DOXYINC = "share/doxygen",
        DOXYCSS = "share/doxygen/doxygen.css",
        PACKAGEINC = None
    )

    env.Append(BUILDERS = {
        'Doxygen': doxygen_builder
    })


def exists(env):
    return True
