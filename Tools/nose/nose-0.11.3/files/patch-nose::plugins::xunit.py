--- nose/plugins/xunit.py.orig	2010-10-01 11:00:23.656196268 +1000
+++ nose/plugins/xunit.py	2010-10-01 11:00:35.606216948 +1000
@@ -59,6 +59,17 @@
     """Escape a string for an XML CDATA section."""
     return xml_safe(cdata).replace(']]>', ']]>]]&gt;<![CDATA[')
 
+def id_split(idval):
+    rx = re.compile('^(.*?)(\(.*\))$')
+    m = rx.match(idval)
+    if m:
+        name, fargs = m.groups()
+        head, tail = name.rsplit(".", 1)
+        return [head, tail+fargs]
+    else:
+        return idval.rsplit(".", 1)
+        
+
 def nice_classname(obj):
     """Returns a nice name for class object or class instance.
 
@@ -190,8 +201,8 @@
             '<testcase classname=%(cls)s name=%(name)s time="%(taken)d">'
             '<%(type)s type=%(errtype)s message=%(message)s><![CDATA[%(tb)s]]>'
             '</%(type)s></testcase>' %
-            {'cls': self._quoteattr('.'.join(id.split('.')[:-1])),
-             'name': self._quoteattr(id.split('.')[-1]),
+            {'cls': self._quoteattr(id_split(id)[0]),
+             'name': self._quoteattr(id_split(id)[-1]),
              'taken': taken,
              'type': type,
              'errtype': self._quoteattr(nice_classname(err[0])),
@@ -210,8 +221,8 @@
             '<testcase classname=%(cls)s name=%(name)s time="%(taken)d">'
             '<failure type=%(errtype)s message=%(message)s><![CDATA[%(tb)s]]>'
             '</failure></testcase>' %
-            {'cls': self._quoteattr('.'.join(id.split('.')[:-1])),
-             'name': self._quoteattr(id.split('.')[-1]),
+            {'cls': self._quoteattr(id_split(id)[0]),
+             'name': self._quoteattr(id_split(id)[-1]),
              'taken': taken,
              'errtype': self._quoteattr(nice_classname(err[0])),
              'message': self._quoteattr(exc_message(err)),
@@ -227,7 +238,7 @@
         self.errorlist.append(
             '<testcase classname=%(cls)s name=%(name)s '
             'time="%(taken)d" />' %
-            {'cls': self._quoteattr('.'.join(id.split('.')[:-1])),
-             'name': self._quoteattr(id.split('.')[-1]),
+            {'cls': self._quoteattr(id_split(id)[0]),
+             'name': self._quoteattr(id_split(id)[-1]),
              'taken': taken,
              })
