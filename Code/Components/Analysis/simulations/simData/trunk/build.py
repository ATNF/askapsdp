# @file
from recursivebuild.thirdpartybuilder import ThirdPartyBuilder
from recursivebuild import run

class MyBuilder(ThirdPartyBuilder):
    def _clean(self):
        run("/bin/rm -f data/*uJy_*")

    def _build(self):
        run("/bin/sh createSourceLists.sh")

builder = MyBuilder(pkgname=".")

builder.build()
