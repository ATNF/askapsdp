--- casa/BasicSL/Complexfwd.h.orig	2014-01-28 16:21:40.000000000 +1100
+++ casa/BasicSL/Complexfwd.h	2014-01-28 16:22:07.000000000 +1100
@@ -30,6 +30,7 @@
 
 //# Includes
 
+#include <complex>
 #include <casa/aips.h>
 
 // <summary> Forward declaration complex classes</summary>
@@ -42,10 +43,6 @@
 
 // <group name=Complexfwd>
 
-namespace std {
-  template<class T> class complex;
-}
-
 namespace casa { //# NAMESPACE CASA - BEGIN
 
 typedef std::complex<Float>  Complex;
