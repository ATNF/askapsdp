--- src/Outputs/CatalogueSpecification.hh.orig	2015-01-12 13:18:34.000000000 +1100
+++ src/Outputs/CatalogueSpecification.hh	2015-01-12 13:17:17.000000000 +1100
@@ -36,6 +36,9 @@
 
 namespace duchamp {
 
+    class Detection;
+    class FitsHeader;
+
   namespace Catalogues {
 
     class CatalogueSpecification 
@@ -47,6 +50,7 @@
       virtual ~CatalogueSpecification(){};
 
       void addColumn(Column col);
+      void addColumn(std::string type, std::string name, std::string units, int width, int prec, std::string ucd="", std::string datatype="", std::string colID="", std::string extraInfo="");
       Column &column(std::string type){return itsColumnList[itsTypeMap[type]];};
       Column &column(int i){return itsColumnList[i];};
       Column *pCol(int i){return &(itsColumnList[i]);};
@@ -61,6 +65,13 @@
       void setCommentString(std::string comment){itsCommentString = comment;};
       std::string commentString(){return itsCommentString;};
 
+        /// If the named column exists in the specification, check its width using the value
+        template <class T> void check(std::string type, T value);
+        /// If the named column exists in the specification, check its width and precision using the value
+        template <class T> void check(std::string type, T value, bool doPrec);
+
+        void checkAll(std::vector<Detection> &objectList, FitsHeader &head);
+
     protected:
       std::vector<Column> itsColumnList;
       std::map<std::string, int> itsTypeMap;
