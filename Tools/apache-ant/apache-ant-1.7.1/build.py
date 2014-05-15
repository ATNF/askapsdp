import os

from askapdev.rbuild.builders import Data as Builder
    
ASKAP_ROOT = os.environ['ASKAP_ROOT']
ANT_DIR = os.path.join(ASKAP_ROOT, 'share', 'ant')

def callback():
    antpth = os.path.join('bin','ant')
    source = os.path.join(ANT_DIR, antpth)
    target = os.path.join(ASKAP_ROOT, antpth)
    if os.path.exists(target):
        os.remove(target)
    os.symlink(source, target)


builder = Builder()
builder.remote_archive = "apache-ant-1.7.1-bin.tar.bz2"
builder.add_postcallback(callback)
builder._installdir = ANT_DIR

builder.build()
