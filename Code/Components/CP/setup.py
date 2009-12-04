from askapdev.rbuild import build

nobuild = {'benchmarks'  : "Not integrated into build system yet."}

for pkg, msg in nobuild.iteritems():
    print("warn: %s no install. %s" % (pkg, msg))

build(['common/trunk/build.py',
       'imager/trunk/build.py',
       'frontend/trunk/build.py',
       'manager/trunk/build.py'
      ])
