--- ms/MeasurementSets/MSConcat.cc.orig	2015-03-06 11:40:36.000000000 +1100
+++ ms/MeasurementSets/MSConcat.cc	2015-03-06 11:39:52.000000000 +1100
@@ -390,6 +390,11 @@
   Vector<Int> obsIds=otherObsId.getColumn();
 
   ScalarColumn<Int>& thisObsId = observationId();
+  const ROArrayColumn<Float>& otherSigmaSp = otherMainCols.sigmaSpectrum();
+  ArrayColumn<Float>& thisSigmaSp = sigmaSpectrum();
+  Bool copySigSp = !(thisSigmaSp.isNull() || otherSigmaSp.isNull()); 
+  copySigSp = copySigSp && thisSigmaSp.isDefined(0) 
+    && otherSigmaSp.isDefined(0);
   const ROArrayColumn<Float>& otherWeightSp = otherMainCols.weightSpectrum();
   ArrayColumn<Float>& thisWeightSp = weightSpectrum();
   Bool copyWtSp = !(thisWeightSp.isNull() || otherWeightSp.isNull()); 
@@ -590,6 +595,7 @@
     thisFlag.put(curRow, otherFlag, r);
     if (copyFlagCat) thisFlagCat.put(curRow, otherFlagCat, r);
     thisFlagRow.put(curRow, otherFlagRow, r);
+    if (copySigSp) thisSigmaSp.put(curRow, otherSigmaSp, r);
     if (copyWtSp) thisWeightSp.put(curRow, otherWeightSp, r);
   } 
 

