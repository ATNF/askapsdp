--- src/FitsIO/subsection.cc.orig	2014-12-12 12:08:01.115267721 +1100
+++ src/FitsIO/subsection.cc	2014-12-12 12:09:24.489174468 +1100
@@ -51,11 +51,11 @@
 
     if(this->flagSubsection){ // if so, then the offsets array is defined.
       int specAxis = wcs->spec;
-      if(specAxis<0) specAxis=2;
       if(specAxis>=wcs->naxis) specAxis = wcs->naxis-1;
       this->xSubOffset = this->pixelSec.getStart(wcs->lng);
       this->ySubOffset = this->pixelSec.getStart(wcs->lat);
-      this->zSubOffset = this->pixelSec.getStart(specAxis);
+      if(specAxis<0)  this->zSubOffset = 0;
+      else            this->zSubOffset = this->pixelSec.getStart(specAxis);
     }
     else{// else they should be 0
       this->xSubOffset = this->ySubOffset = this->zSubOffset = 0;
@@ -71,11 +71,11 @@
     
     if(this->flagSubsection){ // if so, then the offsets array is defined.
       int specAxis = wcs.spec;
-      if(specAxis<0) specAxis=2;
       if(specAxis>=wcs.naxis) specAxis = wcs.naxis-1;
       this->xSubOffset = this->pixelSec.getStart(wcs.lng);
       this->ySubOffset = this->pixelSec.getStart(wcs.lat);
-      this->zSubOffset = this->pixelSec.getStart(specAxis);
+      if(specAxis<0)  this->zSubOffset = 0;
+      else            this->zSubOffset = this->pixelSec.getStart(specAxis);
     }
     else{// else they should be 0
       this->xSubOffset = this->ySubOffset = this->zSubOffset = 0;
