# @file
from askapdev.rbuild.thirdpartybuilder import ThirdPartyBuilder
from askapdev.rbuild import run

class MyBuilder(ThirdPartyBuilder):
    def _clean(self):
        run("/bin/rm -f data/*uJy_*")
        ThirdPartyBuilder._clean(self)

    def _build(self):
        run("/bin/sh createSourceLists.sh")

builder = MyBuilder(pkgname=".")

builder.build()
