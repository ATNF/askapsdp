--- scimath/Functionals/SerialHelper.h.orig	2007-02-28 11:59:51.000000000 +0900
+++ scimath/Functionals/SerialHelper.h	2013-10-30 08:15:36.387009000 +0800
@@ -37,13 +37,11 @@
 
 template <class V>
 void getArrayVal(V &val, int type, const Record& gr, 
-		      const String& name, uInt index=0) 
-    throw (InvalidSerializationError);
+		      const String& name, uInt index=0) ;
 
 template <class V>
 void getArray(Array<V> &val, int type, const Record& gr, 
-		   const String& name) 
-    throw (InvalidSerializationError);
+		   const String& name);
 
 // <summary>
 //
@@ -117,14 +115,12 @@
     //   <li> InvalidSerializationError if "functype" exists but is 
     //          empty or the incorrect type
     // </thrown>
-    Bool getFuncType(String& ftype) const
-	throw (InvalidSerializationError);
+    Bool getFuncType(String& ftype) const;
 
     // ensure that the Function type stored in the given record, <em>gr</em>,
     // matches <em>ftype</em>.  If it does not, an 
     // InvalidSerializationError is thrown.
-    void checkFuncType(const String& ftype) const
-	throw (InvalidSerializationError);
+    void checkFuncType(const String& ftype) const;
 
     // return True if a field with the given <em>name</em> exists
     Bool exists(const String &name) const { return gr.isDefined(name); }
@@ -142,26 +138,25 @@
     //  <li> if the index is out of range.
     // </ul>
     // <group>
-    void get(Bool &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
+    void get(Bool &val, const String& name, uInt index = 0) const;
+
 //      void get(uChar &val, const String& name, uInt index = 0) const
 //  	throw (InvalidSerializationError);
-    void get(Short &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(Int &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(Float &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(Double &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(Complex &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(DComplex &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(String &val, const String& name, uInt index = 0) const
-	throw (InvalidSerializationError);
-    void get(Record &val, const String& name) const
-	throw (InvalidSerializationError);
+    void get(Short &val, const String& name, uInt index = 0) const;
+
+    void get(Int &val, const String& name, uInt index = 0) const;
+
+    void get(Float &val, const String& name, uInt index = 0) const;
+
+    void get(Double &val, const String& name, uInt index = 0) const;
+
+    void get(Complex &val, const String& name, uInt index = 0) const;
+
+    void get(DComplex &val, const String& name, uInt index = 0) const;
+
+    void get(String &val, const String& name, uInt index = 0) const;
+
+    void get(Record &val, const String& name) const;
     // </group>
 
     // Get the <em>index</em>th element of the <em>name</em> field 
@@ -177,24 +172,24 @@
     //  <li> if the index is out of range.
     // </ul>
     // <group>
-    void get(Array<Bool> &val, const String& name) const
-	throw (InvalidSerializationError);
+    void get(Array<Bool> &val, const String& name) const;
+
 //      void get(Array<uChar &val, const String& name) const
 //  	throw (InvalidSerializationError);
-    void get(Array<Short> &val, const String& name) const
-	throw (InvalidSerializationError);
-    void get(Array<Int> &val, const String& name) const
-	throw (InvalidSerializationError);
-    void get(Array<Float> &val, const String& name) const
-	throw (InvalidSerializationError);
-    void get(Array<Double> &val, const String& name) const
-	throw (InvalidSerializationError);
-    void get(Array<Complex> &val, const String& name) const
-	throw (InvalidSerializationError);
-    void get(Array<DComplex> &val, const String& name) const
-	throw (InvalidSerializationError);
-    void get(Array<String> &val, const String& name) const
-	throw (InvalidSerializationError);
+    void get(Array<Short> &val, const String& name) const;
+
+    void get(Array<Int> &val, const String& name) const;
+
+    void get(Array<Float> &val, const String& name) const;
+
+    void get(Array<Double> &val, const String& name) const;
+
+    void get(Array<Complex> &val, const String& name) const;
+
+    void get(Array<DComplex> &val, const String& name) const;
+
+    void get(Array<String> &val, const String& name) const;
+
     // </group>
 
     SerialHelper& operator=(const SerialHelper& other) { 
