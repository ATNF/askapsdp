from recursivebuild import build

nobuild = {'accelerators': "Not integrated into build system yet.",
           'benchmarks'  : "Not integrated into build system yet.",
           'config'      : "minicp config does not really belong here?!",
           'mwcontrol'   : "Obsolete - wiki/AS05_Infrastructure/PackageStatus."}
for pkg, msg in nobuild.iteritems():
    print("warn: %s no install. %s" % (pkg, msg))

build([
       'mwcommon/trunk/build.py',
       'askapparallel/trunk/build.py',
       ])
