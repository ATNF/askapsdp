--- scimath/Fitting/FitGaussian.h.orig	2013-05-24 12:32:53.000000000 +1000
+++ scimath/Fitting/FitGaussian.h	2013-05-24 12:28:59.000000000 +1000
@@ -177,6 +177,9 @@
                 T maximumRMS = 1.0, uInt maxiter = 1024, 
                 T convcriteria = 0.0001);
 
+  Matrix<T> solution(){return itsSolutionParameters;};
+  Matrix<T> errors(){return itsSolutionErrors;};
+
   // Internal function for ensuring that parameters stay within their stated
   // domains (see <src>Gaussian2D</src> and <src>Gaussian3D</src>.)
   void correctParameters(Matrix<T>& parameters);
@@ -215,6 +218,11 @@
 
   //Find the number of unmasked parameters to be fit
   uInt countFreeParameters();
+
+  // The solutions to the fit
+  Matrix<T> itsSolutionParameters;
+  // The errors on the solution parameters
+  Matrix<T> itsSolutionErrors;
 };
 
 
