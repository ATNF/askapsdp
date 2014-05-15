--- numpy/distutils/system_info.py.orig	2014-02-10 13:11:47.000000000 +1100
+++ numpy/distutils/system_info.py	2014-02-10 13:13:12.000000000 +1100
@@ -1290,7 +1290,7 @@
 
     def calc_info(self):
 
-        if sys.platform=='darwin' and not os.environ.get('ATLAS',None):
+        if sys.platform=='darwin' and not os.environ.get('ASKAP_ROOT',None):
             args = []
             link_args = []
             if get_platform()[-4:] == 'i386' or 'intel' in get_platform() or \
