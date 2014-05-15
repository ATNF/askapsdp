--- include/cppunit/extensions/TestFixtureFactory.h.orig	2007-10-18 18:45:11.000000000 +0200
+++ include/cppunit/extensions/TestFixtureFactory.h	2007-10-18 19:01:40.000000000 +0200
@@ -16,8 +16,12 @@
 class TestFixtureFactory
 {
 public:
+  //! Virtual destructor.
+  virtual ~TestFixtureFactory() 
+  {        
+  }
   //! Creates a new TestFixture instance.
-  virtual TestFixture *makeFixture() =0;
+  virtual TestFixture *makeFixture() = 0;
 };
 
 
