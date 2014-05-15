--- src/Exception.cc.orig	2013-11-25 11:44:20.000000000 +1100
+++ src/Exception.cc	2013-11-25 11:44:44.000000000 +1100
@@ -26,11 +26,8 @@
 #include <Common/Exception.h>
 #include <cstdlib>    // for abort()
 #include <iostream>
-
-#ifndef __GNUG_
 # include <cstdio>    // for fputs()
 # include <typeinfo>  // for typeid()
-#endif
 
 namespace LOFAR 
 {
@@ -44,13 +41,9 @@
     // twice, because a rethrow was attempted without an active exception.
     static bool terminating;
     if (terminating) {
-#ifdef __GNUG__
-      __gnu_cxx::__verbose_terminate_handler();
-#else
       // fputs() is the only safe way to print to stderr when low on memory
       fputs("terminate called recursively\n", stderr);
       abort();
-#endif
     }
     terminating = true;
     
@@ -81,9 +74,6 @@
           cerr << Backtrace() << endl;
         } catch (...) {}
 #endif
-#ifdef __GNUG__
-        __gnu_cxx::__verbose_terminate_handler();
-#else
         // Rethrow once more to separate std::exception from other exceptions.
         try {
           throw;
@@ -94,7 +84,6 @@
 	    cerr << typeid(e).name() << ": " << e.what() << endl;
 	  } catch (...) {}
         }
-#endif
       }
     }
     abort();
