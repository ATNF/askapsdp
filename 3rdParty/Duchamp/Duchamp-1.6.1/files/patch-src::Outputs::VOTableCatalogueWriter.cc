--- src/Outputs/VOTableCatalogueWriter.cc.orig	2015-02-26 09:21:56.000000000 +1100
+++ src/Outputs/VOTableCatalogueWriter.cc	2015-02-26 08:58:53.000000000 +1100
@@ -75,7 +75,7 @@
 
 	    this->writeSTC();
 
-	    this->itsFileStream<<"  <RESOURCE name=\"Duchamp Output\">\n";
+	    this->itsFileStream<<"  <RESOURCE name=\""<< this->itsResourceName <<"\">\n";
 	    this->itsFileStream<<"    <TABLE name=\""<<this->itsTableName<<"\">\n";
 	    this->itsFileStream<<"      <DESCRIPTION>"<<this->itsTableDescription<<"</DESCRIPTION>\n";
 
@@ -89,8 +89,7 @@
 	    this->itsFileStream<<"<?xml version=\"1.0\"?>\n";
 	    this->itsFileStream<<"<VOTABLE version=\"1.3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
 			       <<" xmlns=\"http://www.ivoa.net/xml/VOTable/v1.3\"\n"
-			       <<" xmlns:stc=\"http://www.ivoa.net/xml/STC/v1.30\"\n"
-			       <<" xsi:schemaLocation=\"http://www.ivoa.net/xml/VOTable/VOTable/v1.1\" >\n";
+			       <<" xmlns:stc=\"http://www.ivoa.net/xml/STC/v1.30\" >\n";
 	}
     }
 
