--- scimath/Functionals/SerialHelper.cc.orig	2013-10-30 08:21:58.507337000 +0800
+++ scimath/Functionals/SerialHelper.cc	2013-10-30 08:24:03.431572000 +0800
@@ -39,7 +39,6 @@
 };
 
 Bool SerialHelper::getFuncType(String& ftype) const 
-    throw (InvalidSerializationError)
 {
     if (! gr.isDefined(FUNCTYPE)) return False;
 
@@ -55,7 +54,6 @@
 }
 
 void SerialHelper::checkFuncType(const String& ftype) const 
-    throw (InvalidSerializationError)
 {
     String thistype;
     if (! getFuncType(thistype))
@@ -66,7 +64,7 @@
 }
 
 template <> void getArrayVal<Bool>(Bool& val,     Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -88,7 +86,7 @@
     }
 }
 template <> void getArrayVal<Short>(Short& val,    Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -110,7 +108,7 @@
     }
 }
 template <> void getArrayVal<Int>(Int& val,      Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -132,7 +130,7 @@
     }
 }
 template <> void getArrayVal<Float>(Float& val,    Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -154,7 +152,7 @@
     }
 }
 template <> void getArrayVal<Double>(Double& val,   Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -176,7 +174,7 @@
     }
 }
 template <> void getArrayVal<Complex>(Complex& val,  Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -198,7 +196,7 @@
     }
 }
 template <> void getArrayVal<DComplex>(DComplex& val, Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -220,7 +218,7 @@
     }
 }
 template <> void getArrayVal<String>(String& val,   Int, const Record& gr, 
-			                  const String& name, uInt index)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                  const String& name, uInt index)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -243,7 +241,7 @@
 }
 
 template <> void getArray<Bool>(Array<Bool>& val,     Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -254,7 +252,7 @@
     val = gr.asArrayBool(RecordFieldId(name));
 }
 template <> void getArray<Short>(Array<Short>& val,    Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -265,7 +263,7 @@
     val = gr.asArrayShort(RecordFieldId(name));
 }
 template <> void getArray<Int>(Array<Int>& val,      Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -276,7 +274,7 @@
     val = gr.asArrayInt(RecordFieldId(name));
 }
 template <> void getArray<Float>(Array<Float>& val,    Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -287,7 +285,7 @@
     val = gr.asArrayFloat(RecordFieldId(name));
 }
 template <> void getArray<Double>(Array<Double>& val,   Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -298,7 +296,7 @@
     val = gr.asArrayDouble(RecordFieldId(name));
 }
 template <> void getArray<Complex>(Array<Complex>& val,  Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -309,7 +307,7 @@
     val = gr.asArrayComplex(RecordFieldId(name));
 }
 template <> void getArray<DComplex>(Array<DComplex>& val, Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -320,7 +318,7 @@
     val = gr.asArrayDComplex(RecordFieldId(name));
 }
 template <> void getArray<String>(Array<String>& val,   Int, const Record& gr, 
-			                      const String& name)WHATEVER_SUN_EXCEPTSPEC(InvalidSerializationError)
+			                      const String& name)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
@@ -333,13 +331,11 @@
 
 
 void SerialHelper::get(Bool &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtBOOL, gr, name, index);
 }
 
 void SerialHelper::get(String &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtSTRING, gr, name, index);
 }
@@ -351,91 +347,76 @@
 //  }
 
 void SerialHelper::get(Short &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtSHORT, gr, name, index);
 }
 
 void SerialHelper::get(Int &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtINT, gr, name, index);
 }
 
 void SerialHelper::get(Float &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtFLOAT, gr, name, index);
 }
 
 void SerialHelper::get(Double &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtDOUBLE, gr, name, index);
 }
 
 void SerialHelper::get(Complex &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtCOMPLEX, gr, name, index);
 }
 
 void SerialHelper::get(DComplex &val, const String& name, uInt index) const
-    throw (InvalidSerializationError)
 {
     getArrayVal(val, SerialHelper::shtDCOMPLEX, gr, name, index);
 }
 
 void SerialHelper::get(Array<Bool> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtBOOL, gr, name);
 }
 
 void SerialHelper::get(Array<Short> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtSHORT, gr, name);
 }
 
 void SerialHelper::get(Array<Int> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtINT, gr, name);
 }
 
 void SerialHelper::get(Array<Float> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtFLOAT, gr, name);
 }
 
 void SerialHelper::get(Array<Double> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtDOUBLE, gr, name);
 }
 
 void SerialHelper::get(Array<Complex> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtCOMPLEX, gr, name);
 }
 
 void SerialHelper::get(Array<DComplex> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtDCOMPLEX, gr, name);
 }
 
 void SerialHelper::get(Array<String> &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     getArray(val, SerialHelper::shtSTRING, gr, name);
 }
 
 void SerialHelper::get(Record &val, const String& name) const
-    throw (InvalidSerializationError)
 {
     if (! gr.isDefined(name)) throw FieldNotFoundError(name);
     //std::cerr << name << " "<< gr.dataType(RecordFieldId(name)) << endl;
