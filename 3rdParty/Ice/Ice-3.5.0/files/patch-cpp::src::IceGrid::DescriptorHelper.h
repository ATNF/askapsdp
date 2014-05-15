--- cpp/src/IceGrid/DescriptorHelper.h.orig	2013-11-26 17:37:13.000000000 +1100
+++ cpp/src/IceGrid/DescriptorHelper.h	2013-11-26 17:38:39.000000000 +1100
@@ -247,7 +247,6 @@
     ServerInstanceHelper(const ServerInstanceDescriptor&, const Resolver&, bool);
     ServerInstanceHelper(const ServerDescriptorPtr&, const Resolver&, bool);
     
-    void operator=(const ServerInstanceHelper&);
     bool operator==(const ServerInstanceHelper&) const;
     bool operator!=(const ServerInstanceHelper&) const;
 
@@ -265,7 +264,7 @@
 
     void init(const ServerDescriptorPtr&, const Resolver&, bool);
 
-    const ServerInstanceDescriptor _def;
+    ServerInstanceDescriptor _def;
     std::string _id;
     ServerInstanceDescriptor _instance;
 
