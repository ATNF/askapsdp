--- casa/Arrays/MaskArrMath.tcc.orig	2014-09-16 16:05:03.000000000 +1000
+++ casa/Arrays/MaskArrMath.tcc	2014-09-16 16:05:27.000000000 +1000
@@ -1582,6 +1582,12 @@
     return medval;
 }
 
+template<class T> T madfm(const MaskedArray<T> &a, Bool sorted, Bool takeEvenMean)
+{
+    T med = median(a, sorted, takeEvenMean);
+    MaskedArray<T> absdiff = abs(a - med);
+    return median(absdiff, sorted, takeEvenMean);
+}
 
 template<class T> MaskedArray<T> square(const MaskedArray<T> &left)
 {
@@ -1739,13 +1745,23 @@
   MaskedArray<T> arr (array);
   Array<T> result (resShape);
   DebugAssert (result.contiguousStorage(), AipsError);
+  Array<Bool> resultMask(resShape);
   T* res = result.data();
+  Bool* resMask = resultMask.data();
   // Loop through all data and assemble as needed.
   IPosition blc(ndim, 0);
   IPosition trc(hboxsz);
   IPosition pos(ndim, 0);
   while (True) {
-    *res++ = funcObj (arr(blc,trc));
+//    *res++ = funcObj (arr(blc,trc));
+    MaskedArray<T> subarr (arr(blc,trc));
+    if (subarr.nelementsValid() == 0) {
+      *resMask++ = False;
+      *res++ = T();
+    } else {
+      *resMask++ = True;
+      *res++ = funcObj (arr(blc,trc));
+    }
     uInt ax;
     for (ax=0; ax<ndim; ax++) {
       if (++pos[ax] < resShape[ax]) {
