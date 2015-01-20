--- src/Outputs/CatalogueSpecification.cc.orig	2015-01-12 13:18:34.000000000 +1100
+++ src/Outputs/CatalogueSpecification.cc	2015-01-12 13:17:24.000000000 +1100
@@ -29,7 +29,8 @@
 #include <duchamp/Outputs/CatalogueSpecification.hh>
 #include <duchamp/duchamp.hh>
 #include <duchamp/Outputs/columns.hh>
-#include <duchamp/Outputs/CatalogueSpecification.hh>
+#include <duchamp/Detection/detection.hh>
+#include <duchamp/fitsHeader.hh>
 #include <vector>
 #include <map>
 #include <string>
@@ -60,6 +61,13 @@
       this->setMap();
     }
 
+    void CatalogueSpecification::addColumn(std::string type, std::string name, std::string units, int width, int prec, std::string ucd, std::string datatype, std::string colID, std::string extraInfo)
+    {
+        Column col(type,name,units,width,prec,ucd,datatype,colID,extraInfo);
+        this->addColumn(col);
+    }
+      
+      
       void CatalogueSpecification::setMap()
       {
 	  for(size_t i=0;i<this->itsColumnList.size();i++)
@@ -112,6 +120,99 @@
     }
 
 
+      template <class T> void CatalogueSpecification::check(std::string type, T value)
+      {
+          if(hasColumn(type)){
+              itsColumnList[itsTypeMap[type]].check(value);
+          }
+      }
+      template void CatalogueSpecification::check<int>(std::string type, int value);
+      template void CatalogueSpecification::check<long>(std::string type, long value);
+      template void CatalogueSpecification::check<unsigned int>(std::string type, unsigned int value);
+      template void CatalogueSpecification::check<unsigned long>(std::string type, unsigned long value);
+      template void CatalogueSpecification::check<std::string>(std::string type, std::string value);
+      template void CatalogueSpecification::check<float>(std::string type, float value);
+      template void CatalogueSpecification::check<double>(std::string type, double value);
+      
+      template <class T> void CatalogueSpecification::check(std::string type, T value, bool doPrec)
+      {
+          if(hasColumn(type)){
+              itsColumnList[itsTypeMap[type]].check(value,doPrec);
+          }
+      }
+      template void CatalogueSpecification::check<float>(std::string type, float value, bool doPrec);
+      template void CatalogueSpecification::check<double>(std::string type, double value, bool doPrec);
+
+      void CatalogueSpecification::checkAll(std::vector<Detection> &objectList, FitsHeader &head)
+      {
+          // Now test each object against each new column, ensuring each
+          // column has sufficient width and (in most cases) precision to
+          // accomodate the data.
+          std::vector<Detection>::iterator obj;
+          for(obj = objectList.begin(); obj < objectList.end(); obj++){
+
+              this->check("NUM",obj->getID());
+              this->check("NAME",obj->getName());
+              this->check("X",obj->getXcentre()+obj->getXOffset());
+              this->check("Y",obj->getYcentre()+obj->getYOffset());
+              this->check("Z",obj->getZcentre()+obj->getZOffset());
+              if(head.isWCS()){
+                  this->check("RA",obj->getRAs());
+                  this->check("DEC",obj->getDecs());
+                  this->check("RAJD",obj->getRA());
+                  this->check("DECJD",obj->getDec());
+                  if(head.canUseThirdAxis()){
+                      this->check("VEL",obj->getVel());
+                  }
+                  this->check("MAJ",obj->getMajorAxis());
+                  this->check("MIN",obj->getMinorAxis());
+                  // For the PA column, we don't increase the precision. If
+                  // something is very close to zero position angle, then
+                  // we're happy to call it zero.
+                  this->check("PA",obj->getPositionAngle(),false);
+                  this->check("WRA",obj->getRAWidth());
+                  this->check("WDEC",obj->getDecWidth());
+                  if(head.canUseThirdAxis()){
+                      this->check("W50",obj->getW50());
+                      this->check("W20",obj->getW20());
+                      this->check("WVEL",obj->getVelWidth());
+                  }
+	    
+                  this->check("FINT",obj->getIntegFlux());
+                  if(obj->getIntegFluxError()>0.)
+                      this->check("FINTERR",obj->getIntegFluxError());
+              }
+              this->check("FTOT",obj->getTotalFlux());
+              if(obj->getTotalFluxError()>0.)
+                  this->check("FTOTERR",obj->getTotalFluxError());
+              this->check("FPEAK",obj->getPeakFlux());
+              if(obj->getPeakSNR()>0.)
+                  this->check("SNRPEAK",obj->getPeakSNR());
+              this->check("X1",obj->getXmin()+obj->getXOffset());
+              this->check("X2",obj->getXmax()+obj->getXOffset());
+              this->check("Y1",obj->getYmin()+obj->getYOffset());
+              this->check("Y2",obj->getYmax()+obj->getYOffset());
+              this->check("Z1",obj->getZmin()+obj->getZOffset());
+              this->check("Z2",obj->getZmax()+obj->getZOffset());
+              this->check("NVOX",obj->getSize());
+              this->check("XAV",obj->getXaverage()+obj->getXOffset());
+              this->check("YAV",obj->getYaverage()+obj->getYOffset());
+              this->check("ZAV",obj->getZaverage()+obj->getZOffset());
+              this->check("XCENTROID",obj->getXCentroid()+obj->getXOffset());
+              this->check("YCENTROID",obj->getYCentroid()+obj->getYOffset());
+              this->check("ZCENTROID",obj->getZCentroid()+obj->getZOffset());
+              this->check("XPEAK",obj->getXPeak()+obj->getXOffset());
+              this->check("YPEAK",obj->getYPeak()+obj->getYOffset());
+              this->check("ZPEAK",obj->getZPeak()+obj->getZOffset());
+              this->check("NUMCH",obj->getNumChannels());
+              this->check("SPATSIZE",obj->getSpatialSize());
+
+          }
+
+
+      }
+
+
   }
 
 }
