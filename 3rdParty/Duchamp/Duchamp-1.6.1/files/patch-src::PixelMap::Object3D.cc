--- src/PixelMap/Object3D.cc.orig	2015-01-28 14:33:28.205181742 +1100
+++ src/PixelMap/Object3D.cc	2015-01-28 14:34:09.833546174 +1100
@@ -468,6 +468,7 @@
     }
     this->chanlist.clear();
     this->chanlist = newmap;
+    this->spatialMap.addOffsets(xoff,yoff);
     if(this->numVox>0){
       this->xSum += xoff*numVox;
       this->xmin += xoff; this->xmax += xoff;
