--- setup.py	2012-10-08 13:46:54.000000000 +1100
+++ setup.py.new	2013-01-24 10:48:13.000000000 +1100
@@ -3,8 +3,6 @@
 import os
 import sys
 
-from distribute_setup import use_setuptools
-use_setuptools()
 from setuptools import setup, Extension
 
 if not hasattr(sys, "hexversion") or sys.hexversion < 0x02040000:
