--- include/Common/LofarTypedefs.h.orig	2009-09-01 13:12:05.000000000 +0200
+++ include/Common/LofarTypedefs.h	2009-09-01 13:59:59.000000000 +0200
@@ -25,6 +25,12 @@
 
 // \file
 
+#ifndef HAVE_BOOST
+# define HAVE_BOOST 1
+#endif
+#ifndef AUTO_FUNCTION_NAME
+# define AUTO_FUNCTION_NAME __FUNCTION__
+#endif
 
 namespace LOFAR {
 
