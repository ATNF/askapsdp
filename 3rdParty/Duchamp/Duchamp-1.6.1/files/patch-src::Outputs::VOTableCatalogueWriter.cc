--- src/Outputs/VOTableCatalogueWriter.cc.orig	2015-01-23 14:34:36.000000000 +1100
+++ src/Outputs/VOTableCatalogueWriter.cc	2015-01-23 14:35:09.000000000 +1100
@@ -89,8 +89,7 @@
 	    this->itsFileStream<<"<?xml version=\"1.0\"?>\n";
 	    this->itsFileStream<<"<VOTABLE version=\"1.3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
 			       <<" xmlns=\"http://www.ivoa.net/xml/VOTable/v1.3\"\n"
-			       <<" xmlns:stc=\"http://www.ivoa.net/xml/STC/v1.30\"\n"
-			       <<" xsi:schemaLocation=\"http://www.ivoa.net/xml/VOTable/VOTable/v1.1\" >\n";
+			       <<" xmlns:stc=\"http://www.ivoa.net/xml/STC/v1.30\" >\n";
 	}
     }
 
