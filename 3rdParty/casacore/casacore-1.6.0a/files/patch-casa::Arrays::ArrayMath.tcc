--- casa/Arrays/ArrayMath.tcc.orig	2014-06-04 14:36:22.000000000 +1000
+++ casa/Arrays/ArrayMath.tcc	2014-06-04 14:52:34.000000000 +1000
@@ -1230,6 +1230,16 @@
 // <thrown>
 //    </item> ArrayError
 // </thrown>
+template<class T> T madfm(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool takeEvenMean, Bool inPlace)
+{
+    T med = median(a, tmp, sorted, takeEvenMean, inPlace);
+    Array<T> absdiff = abs(a - med);
+    return median(absdiff, tmp, sorted, takeEvenMean, inPlace);
+}
+
+// <thrown>
+//    </item> ArrayError
+// </thrown>
 template<class T> T fractile(const Array<T> &a, Block<T>& tmp, Float fraction,
 			     Bool sorted, Bool inPlace)
 {
