from askapdev.rbuild import build

nobuild = {'benchmarks'  : "Not integrated into build system yet."}

for pkg, msg in nobuild.iteritems():
    print("warn: %s no install. %s" % (pkg, msg))

build(['mwcommon/trunk/build.py',
       'askapparallel/trunk/build.py',
       'imager/trunk/build.py',
      ])
