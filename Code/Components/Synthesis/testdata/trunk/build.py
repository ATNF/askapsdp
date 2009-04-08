# @file
from askapdev.rbuild.thirdpartybuilder import ThirdPartyBuilder
from askapdev.rbuild import run

class MyBuilder(ThirdPartyBuilder):
    def _build(self):
        for fn in ['measdata', 'testdataset']:
            run("tar -xjf %s.tar.bz2" % fn)

builder = MyBuilder(pkgname=".")
builder.add_extra_clean_targets('data')
builder.add_extra_clean_targets('testdataset.ms')

builder.build()
