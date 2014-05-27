--- src/Outputs/KarmaAnnotationWriter.cc.orig	2014-05-26 12:02:59.000000000 +1000
+++ src/Outputs/KarmaAnnotationWriter.cc	2014-05-27 12:13:27.000000000 +1000
@@ -118,9 +118,13 @@
   {
     if(this->itsOpenFlag){
       if(x.size()==y.size()){
-	this->itsFileStream << "CLINES";
-	for(size_t i=0;i<x.size();i++) this->itsFileStream <<" " << x[i] << " " << y[i];
-	this->itsFileStream << "\n";
+	// this->itsFileStream << "CLINES";
+	// for(size_t i=0;i<x.size();i++) this->itsFileStream <<" " << x[i] << " " << y[i];
+	// this->itsFileStream << "\n";
+	  
+	  for(size_t i=0;i<x.size()-1;i++) this->line(x[i],x[i+1],y[i],y[i+1]);
+	  this->line(x[x.size()-1],x[0],y[x.size()-1],y[0]);
+
       }
     }
   }
