--- include/Blob/BlobStreamable.h.orig	2013-10-31 10:30:43.780389000 +0800
+++ include/Blob/BlobStreamable.h	2013-10-31 10:31:00.962600000 +0800
@@ -75,7 +75,7 @@
 
   // Factory that can be used to generate new BlobStreamable objects.
   // The factory is defined as a singleton.
-  typedef Singleton< ObjectFactory< BlobStreamable(), string > >
+  typedef Singleton< ObjectFactory< BlobStreamable*(), string > >
   BlobStreamableFactory;
 
   // @}
