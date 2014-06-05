--- casa/Arrays/ArrayMath.h.orig	2014-06-05 10:43:27.000000000 +1000
+++ casa/Arrays/ArrayMath.h	2014-06-05 10:44:32.000000000 +1000
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
@@ -679,6 +695,48 @@
 			     Bool sorted = False, Bool inPlace = False);
 
 
+// Return the inter-hexile range of an array.  
+// This is the full range between the bottom sixth and the top sixth
+// of ordered array values. "The semi-interhexile range is very nearly
+// equal to the rms for a Gaussian distribution, but it is much less
+// sensitive to the tails of extended distributions." (Condon et al
+// 1998)
+// <group>
+template<class T> T interHexileRange(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace = False);
+template<class T> inline T interHexileRange(const Array<T> &a)
+{
+    return interHexileRange(a, False, False);
+}
+template<class T> inline T interHexileRange(const Array<T> &a, Bool sorted)
+{
+    return interHexileRange(a, sorted, False);
+}
+template<class T> T interHexileRange(const Array<T> &a, Bool sorted, Bool inPlace = False)
+{
+    Block<T> tmp; return interHexileRange(a, tmp, sorted, inPlace);
+}
+// </group>
+
+
+// Return the inter-quartile range of an array.  
+// This is the full range between the bottom quarter and the top
+// quarter of ordered array values.
+// <group>
+template<class T> T interQuartileRange(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace = False);
+template<class T> inline T interQuartileRange(const Array<T> &a)
+{
+    return interQuartileRange(a, False, False);
+}
+template<class T> inline T interQuartileRange(const Array<T> &a, Bool sorted)
+{
+    return interQuartileRange(a, sorted, False);
+}
+template<class T> T interQuartileRange(const Array<T> &a, Bool sorted, Bool inPlace = False)
+{
+    Block<T> tmp; return interQuartileRange(a, tmp, sorted, inPlace);
+}
+// </group>
+
 // Methods for element-by-element scaling of complex and real.
 // Note that Complex and DComplex are typedefs for std::complex.
 //<group>
