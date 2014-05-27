--- src/Detection/detection.hh.orig	2014-05-27 13:41:47.555383258 +1000
+++ src/Detection/detection.hh	2014-05-27 13:41:58.455383522 +1000
@@ -109,7 +109,7 @@
     /// @brief Set the values of the axis offsets from the cube. 
     void   setOffsets(Param &par); 
 
-      using Object3D::addOffsets;  //tell the compiler we want both the addOffsets from Object3D *and* Detection
+//      using Object3D::addOffsets;  //tell the compiler we want both the addOffsets from Object3D *and* Detection
 
     /// @brief Add the offset values to the pixel locations 
    void   addOffsets(size_t xoff, size_t yoff, size_t zoff){
