--- src/main/cpp/stringhelper.cpp.orig	2014-02-14 12:43:02.000000000 +1100
+++ src/main/cpp/stringhelper.cpp	2014-02-14 12:43:24.000000000 +1100
@@ -30,6 +30,8 @@
 #include <cctype>
 #include <apr.h>
 
+#include <stdlib.h>
+#include <cstdio>
 
 using namespace log4cxx;
 using namespace log4cxx::helpers;
