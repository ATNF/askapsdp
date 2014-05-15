--- lattices/Lattices/LatticeStatistics.tcc.orig	2014-01-28 17:32:18.000000000 +1100
+++ lattices/Lattices/LatticeStatistics.tcc	2014-01-28 17:33:32.000000000 +1100
@@ -1542,7 +1542,7 @@
     const Int n1 = d.nelements();
 
     for (Int i=0; i<n1; i++) {
-      if (real(n(i)) > 0.5) {
+      if (std::real(n(i)) > 0.5) {
         if (init) {
           dMin = d(i);
           dMax = d(i);
@@ -2200,7 +2200,7 @@
     plotter.page();
 
     if (nL>0) {
-      plotter.swin(real(xMin), real(xMax), real(yLMin), real(yLMax));
+      plotter.swin(std::real(xMin), std::real(xMax), std::real(yLMin), std::real(yLMax));
       if (nR>0) {
         plotter.box("BCNST", 0.0, 0, "BNST", 0.0, 0);
       } else {
@@ -2308,7 +2308,7 @@
 
     i = -1;
     if (nR>0) {
-      plotter.swin(real(xMin), real(xMax), real(yRMin), real(yRMax));
+      plotter.swin(std::real(xMin), std::real(xMax), std::real(yRMin), std::real(yRMax));
       plotter.sci (1); 
       if (nL>0) {
         plotter.box("", 0.0, 0, "CMST", 0.0, 0);
@@ -2397,12 +2397,12 @@
       yb = result(Slice(4,4));
       Float dy = yb(1) - yb(0);
 
-      Float mx = real(xMin) + dx;
+      Float mx = std::real(xMin) + dx;
       Float my;
       if (nR > 0) {
-        my = real(yRMax) + 0.5*dy;
+        my = std::real(yRMax) + 0.5*dy;
       } else {
-        my = real(yLMax) + 0.5*dy;
+        my = std::real(yLMax) + 0.5*dy;
       }
 
       Int tbg;
@@ -2459,8 +2459,8 @@
   //  Bool     False if didn't find another valid datum
   {
     for (uInt i=iStart; i<n; i++) {
-      if ( (findGood && real(mask(i))>0.5) ||
-           (!findGood && real(mask(i))<0.5) ) {
+      if ( (findGood && std::real(mask(i))>0.5) ||
+           (!findGood && std::real(mask(i))<0.5) ) {
         iFound = i;
         return True;
       }
@@ -2705,7 +2705,7 @@
         pStoreLattice_p->getSlice(stats, pos, shape, IPosition(1,1));
    
         pos(0) = NPTS;
-        if (Int(real(stats(pos))+0.1) > 0) {
+        if (Int(std::real(stats(pos))+0.1) > 0) {
           someGoodPointsValue_p = True;
         } else {
           someGoodPointsValue_p = False;
@@ -2737,7 +2737,7 @@
 
         for (pixelIterator.reset(); !pixelIterator.atEnd(); pixelIterator++) {
           for (Int i=0; i<n1; i++) {
-            if (Int(real(pixelIterator.matrixCursor()(i,NPTS))+0.1) > 0) {
+            if (Int(std::real(pixelIterator.matrixCursor()(i,NPTS))+0.1) > 0) {
               someGoodPointsValue_p = True;
               return someGoodPointsValue_p;
             }
