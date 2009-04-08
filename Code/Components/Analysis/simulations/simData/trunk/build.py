# @file
from askapdev.rbuild.builders import Builder
from askapdev.rbuild import run

class MyBuilder(Builder):
    def _clean(self):
        run("/bin/rm -f data/*uJy_*")
        Builder._clean(self)

    def _build(self):
        run("/bin/sh createSourceLists.sh")

builder = MyBuilder(pkgname=".")

builder.build()
