--- src/main/include/log4cxx/helpers/simpledateformat.h.orig	2013-11-06 13:29:48.000000000 +1100
+++ src/main/include/log4cxx/helpers/simpledateformat.h	2013-11-06 13:30:21.000000000 +1100
@@ -27,10 +27,9 @@
 
 #include <log4cxx/helpers/dateformat.h>
 #include <vector>
+#include <locale>
 #include <time.h>
 
-namespace std { class locale; }
-
 namespace log4cxx
 {
         namespace helpers
