--- casa/BasicSL/Complex.h.orig	2014-01-28 18:08:13.000000000 +1100
+++ casa/BasicSL/Complex.h	2014-01-28 18:08:48.000000000 +1100
@@ -341,13 +341,20 @@
 // Define real & complex conjugation for non-complex types
 // and put comparisons into std namespace.
 namespace std { 
+
+// This is a short-term workaround. The problem is these functions are
+// defined in C++11. Casacore should have not put these in the std
+// namespace. Now, on Mac OSX Mavericks, even though we don't compile
+// with C++11 support, the standard library still has these functions.
+#if __clang_major__ != 5
   inline float  conj(float  x) { return x; }
   inline double conj(double x) { return x; }
   inline float  real(float  x) { return x; }
   inline double real(double x) { return x; }
   inline float  imag(float   ) { return 0; }
   inline double imag(double  ) { return 0; }
-  
+#endif
+
   using casa::operator>;
   using casa::operator>=;
   using casa::operator<;
