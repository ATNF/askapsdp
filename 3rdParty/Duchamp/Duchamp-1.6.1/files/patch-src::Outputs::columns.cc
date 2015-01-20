--- src/Outputs/columns.cc.orig	2015-01-12 13:18:34.000000000 +1100
+++ src/Outputs/columns.cc	2015-01-12 13:12:46.000000000 +1100
@@ -419,71 +419,8 @@
       newset.column("FTOTERR").setUnits("[" + head.getFluxUnits() + "]");
       newset.column("FPEAK").setUnits("[" + head.getFluxUnits() + "]");
       
-
-      // Now test each object against each new column, ensuring each
-      // column has sufficient width and (in most cases) precision to
-      // accomodate the data.
-      std::vector<Detection>::iterator obj;
-      for(obj = objectList.begin(); obj < objectList.end(); obj++){
-
-	newset.column("NUM").check(obj->getID());
-	newset.column("NAME").check(obj->getName());
-	newset.column("X").check(obj->getXcentre()+obj->getXOffset());
-	newset.column("Y").check(obj->getYcentre()+obj->getYOffset());
-	newset.column("Z").check(obj->getZcentre()+obj->getZOffset());
-	if(head.isWCS()){
-	    newset.column("RA").check(obj->getRAs());
-	    newset.column("DEC").check(obj->getDecs());
-	    newset.column("RAJD").check(obj->getRA());
-	    newset.column("DECJD").check(obj->getDec());
-	    if(head.canUseThirdAxis()){
-		newset.column("VEL").check(obj->getVel());
-	    }
-	    newset.column("MAJ").check(obj->getMajorAxis());
-	    newset.column("MIN").check(obj->getMinorAxis());
-	    // For the PA column, we don't increase the precision. If
-	    // something is very close to zero position angle, then
-	    // we're happy to call it zero.
-	    newset.column("PA").check(obj->getPositionAngle(),false);
-	    newset.column("WRA").check(obj->getRAWidth());
-	    newset.column("WDEC").check(obj->getDecWidth());
-	    if(head.canUseThirdAxis()){
-		newset.column("W50").check(obj->getW50());
-		newset.column("W20").check(obj->getW20());
-		newset.column("WVEL").check(obj->getVelWidth());
-	    }
-	    
-	    newset.column("FINT").check(obj->getIntegFlux());
-	    if(obj->getIntegFluxError()>0.)
-		newset.column("FINTERR").check(obj->getIntegFluxError());
-	}
-	newset.column("FTOT").check(obj->getTotalFlux());
-	if(obj->getTotalFluxError()>0.)
-	    newset.column("FTOTERR").check(obj->getTotalFluxError());
-	newset.column("FPEAK").check(obj->getPeakFlux());
-	if(obj->getPeakSNR()>0.)
-	    newset.column("SNRPEAK").check(obj->getPeakSNR());
-	newset.column("X1").check(obj->getXmin()+obj->getXOffset());
-	newset.column("X2").check(obj->getXmax()+obj->getXOffset());
-	newset.column("Y1").check(obj->getYmin()+obj->getYOffset());
-	newset.column("Y2").check(obj->getYmax()+obj->getYOffset());
-	newset.column("Z1").check(obj->getZmin()+obj->getZOffset());
-	newset.column("Z2").check(obj->getZmax()+obj->getZOffset());
-	newset.column("NVOX").check(obj->getSize());
-	newset.column("XAV").check(obj->getXaverage()+obj->getXOffset());
-	newset.column("YAV").check(obj->getYaverage()+obj->getYOffset());
-	newset.column("ZAV").check(obj->getZaverage()+obj->getZOffset());
-	newset.column("XCENTROID").check(obj->getXCentroid()+obj->getXOffset());
-	newset.column("YCENTROID").check(obj->getYCentroid()+obj->getYOffset());
-	newset.column("ZCENTROID").check(obj->getZCentroid()+obj->getZOffset());
-	newset.column("XPEAK").check(obj->getXPeak()+obj->getXOffset());
-	newset.column("YPEAK").check(obj->getYPeak()+obj->getYOffset());
-	newset.column("ZPEAK").check(obj->getZPeak()+obj->getZOffset());
-	newset.column("NUMCH").check(obj->getNumChannels());
-	newset.column("SPATSIZE").check(obj->getSpatialSize());
-
-      }
-	  
+      newset.checkAll(objectList,head);
+      
       return newset;
 	  
     }
