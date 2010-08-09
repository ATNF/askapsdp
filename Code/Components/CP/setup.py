from askapdev.rbuild import build

nobuild = {'benchmarks'  : "Not integrated into build system yet."}

for pkg, msg in nobuild.iteritems():
    print("warn: %s no install. %s" % (pkg, msg))

build(['common/current/build.py',
       'mq/current/build.py',
       'icewrapper/current/build.py',
       'imager/current/build.py',
       'manager/current/build.py',
       'ingest/current/build.py',
       'correlatorsim/current/build.py',
      ])
