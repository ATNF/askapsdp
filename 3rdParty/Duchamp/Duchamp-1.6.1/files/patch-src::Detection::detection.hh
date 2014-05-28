--- src/Detection/detection.hh.orig	2014-05-02 10:02:03.000000000 +1000
+++ src/Detection/detection.hh	2014-05-28 13:48:33.817476903 +1000
@@ -109,17 +109,15 @@
     /// @brief Set the values of the axis offsets from the cube. 
     void   setOffsets(Param &par); 
 
-      using Object3D::addOffsets;  //tell the compiler we want both the addOffsets from Object3D *and* Detection
-
     /// @brief Add the offset values to the pixel locations 
-   void   addOffsets(size_t xoff, size_t yoff, size_t zoff){
+   void   addOffsets(long xoff, long yoff, long zoff){
        Object3D::addOffsets(xoff,yoff,zoff);
        xpeak+=xoff; ypeak+=yoff; zpeak+=zoff;
        xCentroid+=xoff; yCentroid+=yoff; zCentroid+=zoff;
     };
 
-      void   addOffsets(){ addOffsets(xSubOffset, ySubOffset, zSubOffset);};
-      void   removeOffsets(size_t xoff, size_t yoff, size_t zoff){ addOffsets(-xoff, -yoff, -zoff);};
+      void   addOffsets(){ Detection::addOffsets(xSubOffset, ySubOffset, zSubOffset);};
+      void   removeOffsets(long xoff, long yoff, long zoff){ addOffsets(-xoff, -yoff, -zoff);};
       void   removeOffsets(){ addOffsets(-xSubOffset, -ySubOffset, -zSubOffset);};
       void   addOffsets(Param &par){setOffsets(par); addOffsets();};
 
