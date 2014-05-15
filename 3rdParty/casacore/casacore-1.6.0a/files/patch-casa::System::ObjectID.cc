--- casa/System/ObjectID.cc.orig	2014-01-28 16:27:12.000000000 +1100
+++ casa/System/ObjectID.cc	2014-01-28 16:27:25.000000000 +1100
@@ -111,7 +111,7 @@
     ostringstream os;
     os << "sequence=" << sequence() << " host=" << hostName() <<
 	" pid=" << pid() << " time=" << creationTime();
-    out = os;
+    out = os.str();
 }
 
 static Bool toInt(Int &val, String &error, const String &in)
