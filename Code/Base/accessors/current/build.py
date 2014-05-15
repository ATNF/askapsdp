# @file
# build script for AutoBuild

from askapdev.rbuild.builders import Scons as Builder
from askapdev.rbuild.utils import run

class MyBuilder(Builder):
    def _build(self):
        for fn in ['measdata', 'testdataset']:
            run("tar -xjf %s.tar.bz2" % fn)
        Builder._build(self)
    
    def _clean(self):
        self.add_extra_clean_targets('data')
        self.add_extra_clean_targets('testdataset.ms')
        Builder._clean(self)
        

builder = MyBuilder(pkgname=".")
builder.build()

