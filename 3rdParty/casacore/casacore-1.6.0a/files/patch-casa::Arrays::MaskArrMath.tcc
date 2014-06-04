--- casa/Arrays/MaskArrMath.tcc.orig	2014-06-04 16:26:45.000000000 +1000
+++ casa/Arrays/MaskArrMath.tcc	2014-06-04 16:25:13.000000000 +1000
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
