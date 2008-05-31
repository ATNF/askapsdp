from recursivebuild import build

nobuild = ['accelerators', 'benchmarks',  'config', 'mwcontrol']
for pkg in nobuild:
    print("warn: No install of %s" % pkg)

build([
       'mwcommon/trunk/build.py',
       'askapparallel/trunk/build.py',
       ])
