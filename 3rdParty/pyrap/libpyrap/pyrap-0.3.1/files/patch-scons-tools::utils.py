--- scons-tools/utils.py.orig	2010-03-31 16:24:05.000000000 +1100
+++ scons-tools/utils.py	2010-03-31 16:24:26.000000000 +1100
@@ -233,7 +233,7 @@
                                  action="store_true", default=False,
                                  help="Enable the HDF5 library")
         env.CLOptions.add_option("--numpy-incdir", dest="numpy_incdir", 
-                                 default=get_numpy_incdir(),
+                                 default="",
                                  action="store", type="string",
                                  help="The directory of the numpy header files")
         env.CLOptions.add_option("--enable-rpath", dest="enable_rpath",
