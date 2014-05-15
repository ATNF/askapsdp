"""
SCons Tool to handle slice2cpp interface generation.
Use this builder and add its output to the sources of 
Static/Shared Object/Library::

  sliced = env.Slice(source="test.ice")
  env.StaticLibrary("xyz.a", [sliced, "other.cc"]

"""
import os
import SCons.Action
import SCons.Builder

def slice_emitter(target, source, env):
    fname = os.path.split(str(source[0]))[-1][:-4]
    fdir = os.path.split(str(source[0]))[0]
    env['SLICESEARCH'] = fdir
    dir = env.Dir("#./$SLICEOUT")
    cpptgt = dir.File(fname+"."+env["SLICESRCEXT"])
    # Don't need .h file target as output, but it is gnerated implictly
    htgt = dir.File(fname+".h")
    return ((cpptgt, htgt), source)

slice_action = SCons.Action.Action('$SLICECOM', '$SLICECOMSTR')

slice_builder = SCons.Builder.Builder(action = slice_action,
                                     src_suffix = '.ice',
                                     emitter = slice_emitter)

def generate(env):
    env["ICE_HOME"] = None
    env["SLICECOM"] = os.path.join("bin", "slice2cpp")
    env["SLICE_FLAGS"] = ""
    env["SLICEOUT"] = "#"
    env["SLICESRCEXT"] = "cc"
    env["SLICEINCLUDE"] = env["SLICEOUT"]
#    env["SLICECOMSTR"] = "Generating c++ interfaces from $SOURCES in $SLICEOUT"
    env["SLICECOM"] = "$ICE_HOME/bin/slice2cpp $SLICEFLAGS --source-ext $SLICESRCEXT -I$SLICESEARCH --include-dir=$SLICEINCLUDE --output-dir=$SLICEOUT $SOURCES"
    env["BUILDERS"]["Slice"] = slice_builder

def exists(env):
    return True
