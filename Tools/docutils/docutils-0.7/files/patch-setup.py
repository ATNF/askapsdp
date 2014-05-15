--- setup.py.old	2012-08-15 13:49:54.083783001 +1000
+++ setup.py	2012-08-15 13:50:36.183783003 +1000
@@ -212,16 +212,16 @@
 def get_extras():
     extras = []
     for module_name, version, attributes in extra_modules:
-        try:
-            module = __import__(module_name)
-            if version and module.__version__ < version:
-                raise ValueError
-            for attribute in attributes or []:
-                getattr(module, attribute)
-            print ('"%s" module already present; ignoring extras/%s.py.'
-                   % (module_name, module_name))
-        except (ImportError, AttributeError, ValueError):
-            extras.append(module_name)
+#        try:
+#            module = __import__(module_name)
+#            if version and module.__version__ < version:
+#                raise ValueError
+#            for attribute in attributes or []:
+#                getattr(module, attribute)
+#            print ('"%s" module already present; ignoring extras/%s.py.'
+#                   % (module_name, module_name))
+#        except (ImportError, AttributeError, ValueError):
+        extras.append(module_name)
     return extras
 
 
