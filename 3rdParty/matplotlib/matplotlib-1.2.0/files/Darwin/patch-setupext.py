--- setupext.py.old	2013-03-28 13:32:02.000000000 +1100
+++ setupext.py	2013-03-28 13:32:14.000000000 +1100
@@ -630,7 +630,7 @@
     'Add the module flags to ft2font extension'
     add_numpy_flags(module)
     if not get_pkgconfig(module, 'freetype2'):
-        module.libraries.extend(['freetype', 'z'])
+        module.libraries.extend(['freetype', 'z', 'bz2'])
         add_base_flags(module)
 
         basedirs = module.include_dirs[:]  # copy the list to avoid inf loop!
@@ -648,6 +648,7 @@
     else:
         add_base_flags(module)
         module.libraries.append('z')
+        module.libraries.append('bz2')
 
     # put this last for library link order
     module.libraries.extend(std_libs)
