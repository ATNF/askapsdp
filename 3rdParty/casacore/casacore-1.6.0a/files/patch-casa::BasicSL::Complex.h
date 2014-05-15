--- casa/BasicSL/Complex.h.orig	2014-01-28 18:05:43.000000000 +1100
+++ casa/BasicSL/Complex.h	2014-01-28 18:05:59.000000000 +1100
@@ -174,13 +174,13 @@
 //# On Linux comparing the norm does not work well in debug mode
 //# for equal values. Therefore they are compared for equality first.
 inline Bool operator>= (const Complex& left, const Complex& right)
-  { return left==right  ?  True : norm(left) >= norm(right); }
+  { return left==right  ?  True : std::norm(left) >= std::norm(right); }
 inline Bool operator>  (const Complex& left, const Complex& right)
-  { return left==right  ?  False : norm(left) > norm(right); }
+  { return left==right  ?  False : std::norm(left) > std::norm(right); }
 inline Bool operator<= (const Complex& left, const Complex& right)
-  { return left==right  ?  True : norm(left) <= norm(right); }
+  { return left==right  ?  True : std::norm(left) <= std::norm(right); }
 inline Bool operator<  (const Complex& left, const Complex& right)
-  { return left==right  ?  False : norm(left) < norm(right); }
+  { return left==right  ?  False : std::norm(left) < std::norm(right); }
 // </group>
 
 
@@ -200,13 +200,13 @@
 // </reviewed>
 // <group name="DComplex comparisons">
 inline Bool operator>= (const DComplex& left, const DComplex& right)
-  { return norm(left) >= norm(right); }
+  { return std::norm(left) >= std::norm(right); }
 inline Bool operator>  (const DComplex& left, const DComplex& right)
-  { return norm(left) >  norm(right); }
+  { return std::norm(left) >  std::norm(right); }
 inline Bool operator<= (const DComplex& left, const DComplex& right)
-  { return norm(left) <= norm(right); }
+  { return std::norm(left) <= std::norm(right); }
 inline Bool operator<  (const DComplex& left, const DComplex& right)
-  { return norm(left) <  norm(right); }
+  { return std::norm(left) <  std::norm(right); }
 // </group>
 
 
