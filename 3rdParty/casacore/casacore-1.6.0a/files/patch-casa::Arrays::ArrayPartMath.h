--- casa/Arrays/ArrayPartMath.h.orig	2014-06-05 10:43:51.000000000 +1000
+++ casa/Arrays/ArrayPartMath.h	2014-06-05 10:56:08.000000000 +1000
@@ -148,10 +148,20 @@
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
 					     Bool inPlace=False);
+template<class T> Array<T> partialInterHexileRanges (const Array<T>& array,
+						     const IPosition& collapseAxes,
+						     Bool inPlace=False);
+template<class T> Array<T> partialInterQuartileRanges (const Array<T>& array,
+						       const IPosition& collapseAxes,
+						       Bool inPlace=False);
 // </group>
 
 
@@ -205,6 +215,19 @@
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
@@ -218,6 +241,28 @@
   Bool     itsInPlace;
   mutable Block<T> itsTmp;
 };
+template<typename T> class InterHexileRangeFunc {
+public:
+  explicit InterHexileRangeFunc(Bool sorted = False, Bool inPlace = False)
+      : itsSorted(sorted), itsInPlace(inPlace) {}
+  Float operator()(const Array<Float>& arr) const
+    { return interHexileRange(arr, itsTmp, itsSorted, itsInPlace); }
+private:
+  Bool     itsSorted;
+  Bool     itsInPlace;
+  mutable Block<Float> itsTmp;
+};
+template<typename T> class InterQuartileRangeFunc {
+public:
+  explicit InterQuartileRangeFunc(Bool sorted = False, Bool inPlace = False)
+      : itsSorted(sorted), itsInPlace(inPlace) {}
+  Float operator()(const Array<Float>& arr) const
+    { return interQuartileRange(arr, itsTmp, itsSorted, itsInPlace); }
+private:
+  Bool     itsSorted;
+  Bool     itsInPlace;
+  mutable Block<Float> itsTmp;
+};
 
 // Apply the given ArrayMath reduction function objects
 // to each box in the array.
