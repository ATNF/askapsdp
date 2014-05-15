--- include/Common/ObjectFactory.h.orig	2013-10-31 10:46:11.504539000 +0800
+++ include/Common/ObjectFactory.h	2013-10-31 10:45:42.490879000 +0800
@@ -115,7 +115,7 @@
   // defaults to 8.
   //
   template<typename Base BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A), typename TypeId>
-  class ObjectFactory<Base (BOOST_PP_ENUM_PARAMS(n, A)), TypeId>
+  class ObjectFactory<Base* (BOOST_PP_ENUM_PARAMS(n, A)), TypeId>
   {
   private:
     // Typedef for the function that creates an instance of a class that
