--- src/Cubes/detectionIO.cc.orig	2015-02-26 09:22:32.000000000 +1100
+++ src/Cubes/detectionIO.cc	2015-02-26 09:23:11.000000000 +1100
@@ -180,6 +180,7 @@
   {
     VOTableCatalogueWriter writer(this->pars().getVOTFile());
     writer.setup(this);
+    writer.setResourceName("Duchamp Output");
     writer.setTableName("Detections");
     writer.setTableDescription("Detected sources and parameters from running the Duchamp source finder.");
     writer.openCatalogue();
