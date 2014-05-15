--- src/main/include/log4cxx/helpers/objectptr.h.orig	2009-08-18 08:16:16.000000000 +1000
+++ src/main/include/log4cxx/helpers/objectptr.h	2009-08-18 08:17:23.000000000 +1000
@@ -28,9 +28,9 @@
 //   switching between the initialization styles.
 //
 #if LOG4CXX_HELGRIND
-#define _LOG4CXX_OBJECTPTR_INIT(x) { exchange(x); 
+#define _LOG4CXX_OBJECTPTR_INIT(x) : ObjectPtrBase() { exchange(x); 
 #else
-#define _LOG4CXX_OBJECTPTR_INIT(x) : p(x) {
+#define _LOG4CXX_OBJECTPTR_INIT(x) : ObjectPtrBase(), p(x) {
 #endif
 
 namespace log4cxx
