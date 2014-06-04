--- casa/Arrays/ArrayPartMath.tcc.orig	2014-06-04 14:53:11.000000000 +1000
+++ casa/Arrays/ArrayPartMath.tcc	2014-06-04 16:17:18.000000000 +1000
@@ -654,6 +654,67 @@
   return result;
 }
 
+template<class T> Array<T> partialMadfms (const Array<T>& array,
+					  const IPosition& collapseAxes,
+					  Bool takeEvenMean,
+					  Bool inPlace)
+{
+  // Need to make shallow copy because operator() is non-const.
+  Array<T> arr = array;
+  // Is there anything to collapse?
+  if (collapseAxes.nelements() == 0) {
+    return (inPlace  ?  array : array.copy());
+  }
+  const IPosition& shape = array.shape();
+  uInt ndim = shape.nelements();
+  if (ndim == 0) {
+    return Array<T>();
+  }
+  // Get the remaining axes.
+  // It also checks if axes are specified correctly.
+  IPosition resAxes = IPosition::otherAxes (ndim, collapseAxes);
+  uInt ndimRes = resAxes.nelements();
+  // Create the result shape.
+  // Create blc and trc to step through the input array.
+  IPosition resShape(ndimRes);
+  IPosition blc(ndim, 0);
+  IPosition trc(shape-1);
+  for (uInt i=0; i<ndimRes; ++i) {
+    resShape[i] = shape[resAxes[i]];
+    trc[resAxes[i]] = 0;
+  }
+  if (ndimRes == 0) {
+    resShape.resize(1);
+    resShape[0] = 1;
+  }
+  Array<T> result (resShape);
+  Bool deleteRes;
+  T* resData = result.getStorage (deleteRes);
+  T* res = resData;
+  Block<T> tmp;
+  // Loop through all data and assemble as needed.
+  IPosition pos(ndimRes, 0);
+  while (True) {
+    *res++ = madfm(arr(blc,trc), tmp, False, takeEvenMean, inPlace);
+    uInt ax;
+    for (ax=0; ax<ndimRes; ax++) {
+      if (++pos(ax) < resShape(ax)) {
+	blc[resAxes[ax]]++;
+	trc[resAxes[ax]]++;
+	break;
+      }
+      pos(ax) = 0;
+      blc[resAxes[ax]] = 0;
+      trc[resAxes[ax]] = 0;
+    }
+    if (ax == ndimRes) {
+      break;
+    }
+  }
+  result.putStorage (resData, deleteRes);
+  return result;
+}
+
 template<class T> Array<T> partialFractiles (const Array<T>& array,
 					     const IPosition& collapseAxes,
 					     Float fraction,
