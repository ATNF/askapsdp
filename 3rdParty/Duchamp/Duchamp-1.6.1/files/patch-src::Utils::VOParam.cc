--- src/Utils/VOParam.cc.orig	2014-10-10 14:58:12.000000000 +1100
+++ src/Utils/VOParam.cc	2014-10-10 14:58:25.000000000 +1100
@@ -90,7 +90,7 @@
 	   << "\" ucd=\"" << this->itsUCD
 	   << "\" datatype=\"" << this->itsDatatype;
     if(this->itsUnits!="")
-      stream << "\" units=\"" << this->itsUnits;
+      stream << "\" unit=\"" << this->itsUnits;
     if(this->itsWidth!=0){
       if(this->itsDatatype=="char")
 	stream << "\" arraysize=\"" << this->itsWidth;
