--- casa/Arrays/ArrayMath.h.orig	2014-06-04 14:36:17.000000000 +1000
+++ casa/Arrays/ArrayMath.h	2014-06-04 14:49:20.000000000 +1000
@@ -667,6 +667,22 @@
 			   Bool takeEvenMean, Bool inPlace = False);
 // </group>
 
+// The median absolute deviation from the median. Interface is as for
+// the median functions
+// <group>
+template<class T> T madfm(const Array<T> &a, Block<T> &tmp, Bool sorted, 
+			  Bool takeEvenMean, Bool inPlace = False);
+template<class T> inline T madfm(const Array<T> &a)
+    { return madfm(a, False, (a.nelements() <= 100), False); }
+template<class T> inline T madfm(const Array<T> &a, Bool sorted)
+    { return madfm(a, sorted, (a.nelements() <= 100), False); }
+template<class T> inline T madfmInPlace(const Array<T> &a, Bool sorted = False)
+    { return madfm(a, sorted, (a.nelements() <= 100), True); }
+template<class T> T madfm(const Array<T> &a, Bool sorted, Bool takeEvenMean, Bool inPlace = False)
+    { Block<T> tmp; return madfm(a, tmp, sorted, takeEvenMean, inPlace); }
+// </group>
+
+
 // Return the fractile of an array.
 // It returns the value at the given fraction of the array.
 // A fraction of 0.5 is the same as the median, be it that no mean of
