--- casa/Arrays/ArrayMath.tcc.orig	2014-06-05 10:43:41.000000000 +1000
+++ casa/Arrays/ArrayMath.tcc	2014-06-05 10:38:42.000000000 +1000
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
@@ -1272,6 +1282,30 @@
     return fracval;
 }
 
+// <thrown>
+//    </item> ArrayError
+// </thrown>
+template<class T> T interHexileRange(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace)
+{
+
+    T hex1 = fractile(a, tmp, 1. / 6., sorted, inPlace);
+    T hex5 = fractile(a, tmp, 5. / 6., sorted, inPlace);
+
+    return (hex5 - hex1);
+}
+
+// <thrown>
+//    </item> ArrayError
+// </thrown>
+template<class T> T interQuartileRange(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace)
+{
+
+    T q1 = fractile(a, tmp, 0.25, sorted, inPlace);
+    T q3 = fractile(a, tmp, 0.75, sorted, inPlace);
+
+    return (q1 - q3);
+}
+
 template<typename T>
 Array<std::complex<T> > makeComplex(const Array<T> &left, const Array<T>& right)
 {
