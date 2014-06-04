--- casa/Arrays/MaskArrMath.h.orig	2014-06-04 16:26:39.000000000 +1000
+++ casa/Arrays/MaskArrMath.h	2014-06-04 16:24:22.000000000 +1000
@@ -423,6 +423,16 @@
 			   Bool takeEvenMean);
 // </group>
 
+// The median absolute deviation from the median. Interface is as for
+// the median functions
+// <group>
+template<class T> inline T madfm(const MaskedArray<T> &a)
+    { return median (a, False, (a.nelements() <= 100)); }
+template<class T> inline T madfm(const MaskedArray<T> &a, Bool sorted)
+    { return median (a, sorted, (a.nelements() <= 100)); }
+template<class T> T madfm(const MaskedArray<T> &a, Bool sorted,
+			   Bool takeEvenMean);
+// </group>
 
 // Returns a MaskedArray where every element is squared.
 template<class T> MaskedArray<T> square(const MaskedArray<T> &val);
@@ -480,6 +490,17 @@
   Bool     itsTakeEvenMean;
   Bool     itsInPlace;
 };
+template<typename T> class MaskedMadfmFunc {
+public:
+  explicit MaskedMadfmFunc(Bool sorted=False, Bool takeEvenMean=True)
+    : itsSorted(sorted), itsTakeEvenMean(takeEvenMean) {}
+  Float operator()(const MaskedArray<Float>& arr) const
+    { return madfm(arr, itsSorted, itsTakeEvenMean); }
+private:
+  Bool     itsSorted;
+  Bool     itsTakeEvenMean;
+  Bool     itsInPlace;
+};
 
 // Apply the given ArrayMath reduction function objects
 // to each box in the array.
