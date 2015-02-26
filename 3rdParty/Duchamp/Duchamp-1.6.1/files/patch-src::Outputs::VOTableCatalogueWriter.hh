--- src/Outputs/VOTableCatalogueWriter.hh.orig	2015-02-26 08:58:03.000000000 +1100
+++ src/Outputs/VOTableCatalogueWriter.hh	2015-02-26 08:58:38.000000000 +1100
@@ -63,10 +63,12 @@
 
 	void writeFooter();
 
+	void setResourceName(std::string s){itsResourceName=s;};
 	void setTableName(std::string s){itsTableName=s;};
 	void setTableDescription(std::string s){itsTableDescription=s;};
 
     protected:
+        std::string itsResourceName;
 	std::string itsTableName;
 	std::string itsTableDescription;
     };
