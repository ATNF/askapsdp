--- components/ComponentModels/GaussianShape.cc.orig	2011-05-05 14:02:33.000000000 +1000
+++ components/ComponentModels/GaussianShape.cc	2011-05-05 14:02:52.000000000 +1000
@@ -238,14 +238,14 @@
   // performance reasons. Both function static and file static variables
   // where considered and rejected for this purpose.
   if (!TwoSidedShape::ok()) return False;
-  if (!near(itsShape.flux(), 1.0, C::dbl_epsilon)) {
+  if (!near(itsShape.flux(), 1.0, C::flt_epsilon)) {
     LogIO logErr(LogOrigin("GaussianCompRep", "ok()"));
     logErr << LogIO::SEVERE << "The internal Gaussian shape does not have"
 	   << " unit area"
            << LogIO::POST;
     return False;
   }
-  if (!near(itsFT.height(), 1.0, C::dbl_epsilon)) {
+  if (!near(itsFT.height(), 1.0, C::flt_epsilon)) {
     LogIO logErr(LogOrigin("GaussianCompRep", "ok()"));
     logErr << LogIO::SEVERE << "The cached Fourier Transform of"
 	   << " the internal Gaussian shape does not have"
