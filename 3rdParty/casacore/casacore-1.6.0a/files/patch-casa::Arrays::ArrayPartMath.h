--- casa/Arrays/ArrayPartMath.h.orig	2014-06-04 14:53:03.000000000 +1000
+++ casa/Arrays/ArrayPartMath.h	2014-06-04 16:16:55.000000000 +1000
@@ -148,6 +148,10 @@
 					   const IPosition& collapseAxes,
 					   Bool takeEvenMean=False,
 					   Bool inPlace=False);
+template<class T> Array<T> partialMadfms (const Array<T>& array,
+					  const IPosition& collapseAxes,
+					  Bool takeEvenMean=False,
+					  Bool inPlace=False);
 template<class T> Array<T> partialFractiles (const Array<T>& array,
 					     const IPosition& collapseAxes,
 					     Float fraction,
@@ -205,6 +209,19 @@
   Bool     itsInPlace;
   mutable Block<T> itsTmp;
 };
+template<typename T> class MadfmFunc {
+public:
+  explicit MadfmFunc(Bool sorted = False, Bool takeEvenMean = True,
+		     Bool inPlace = False)
+      : itsSorted(sorted), itsTakeEvenMean(takeEvenMean), itsInPlace(inPlace) {}
+  Float operator()(const Array<Float>& arr) const
+    { return madfm(arr, itsTmp, itsSorted, itsTakeEvenMean, itsInPlace); }
+private:
+    Bool     itsSorted;
+    Bool     itsTakeEvenMean;
+    Bool     itsInPlace;
+    mutable Block<Float> itsTmp;
+};
 template<typename T> class FractileFunc {
 public:
   explicit FractileFunc (Float fraction,
