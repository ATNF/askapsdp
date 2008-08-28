#include <askap_analysis.h>

#include <analysisutilities/CasaImageUtil.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <casa/aipstype.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageOpener.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/RecordFieldId.h>
#include <string>
#include <stdlib.h>

#include <wcslib/wcs.h>
#include <wcslib/wcsfix.h>

using namespace casa;

namespace askap
{

  namespace analysis
  {

    wcsprm *casaImageToWCS(std::string imageName)
    {
      LatticeBase* lattPtr = ImageOpener::openImage (imageName);
      ImageInterface<Float>* imagePtr = dynamic_cast<ImageInterface<Float>*>(lattPtr);
      IPosition shape=imagePtr->shape();
      long *dim = (long *)shape.storage();
      CoordinateSystem coords=imagePtr->coordinates();
      Record hdr;
      if(!coords.toFITSHeader(hdr,shape,true,'c',true)) throw AskapError("casaImageToWCS: could not read FITS header parameters");
      else std::cout << "casaImageToWCS:  read FITS header:\n" << hdr << "\n";

      struct wcsprm *wcs;
      wcs = (struct wcsprm *)calloc(1,sizeof(struct wcsprm));
      wcs->flag = -1;
      int ndim = shape.size();
      int status = wcsini(1,ndim,wcs);
      if(status)
	ASKAPTHROW(AskapError,"casaImageToWCS: wcsini failed! Code=" << status << ": " << wcs_errmsg[status]);

      RecordFieldId ctypeID("ctype");
      Array<String> ctype = hdr.asArrayString(ctypeID);
      Array<String>::iterator it;
      int i=0;
      for(it=ctype.begin();it!=ctype.end();it++){
	String str= *it;
	strcpy(wcs->ctype[i++],str.c_str());
      }

      RecordFieldId cunitID("cunit");
      Array<String> cunit = hdr.asArrayString(cunitID);
      i=0;
      for(it=cunit.begin();it!=cunit.end();it++) {
	String str = *it;
	strcpy(wcs->cunit[i++],str.c_str());
      }

      std::vector<Double> vals;

      RecordFieldId crpixID("crpix");
      Array<Double>::iterator it2;
      Array<Double> crpix = hdr.asArrayDouble(crpixID);
      crpix.tovector(vals);
      for(uint i=0;i<vals.size();i++) wcs->crpix[i] = double(vals[i]);

      RecordFieldId crvalID("crval");
      Array<Double> crval = hdr.asArrayDouble(crvalID);
      crval.tovector(vals);
      for(uint i=0;i<vals.size();i++) wcs->crval[i] = double(vals[i]);

      RecordFieldId cdeltID("cdelt");
      Array<Double> cdelt = hdr.asArrayDouble(cdeltID);
      cdelt.tovector(vals);
      for(uint i=0;i<vals.size();i++) wcs->cdelt[i] = double(vals[i]);
      
      RecordFieldId crotaID("crota");
      Array<Double> crota = hdr.asArrayDouble(crotaID);
      crota.tovector(vals);
      for(uint i=0;i<vals.size();i++) {
	wcs->crota[i] = double(vals[i]);
	wcs->altlin |= 4;
      }


      RecordFieldId pcID("pc");
      Array<Double> pc = hdr.asArrayDouble(pcID);
      pc.tovector(vals);
      for(uint i=0;i<vals.size();i++) wcs->pc[i] = double(vals[i]);

      RecordFieldId lonpoleID("lonpole"); 
      Double lonpole = hdr.asDouble(lonpoleID);
      wcs->lonpole = double(lonpole);
      
      RecordFieldId equinoxID("equinox"); 
      Double equinox = hdr.asDouble(equinoxID);
      wcs->equinox = double(equinox);
      
      int stat[NWCSFIX];
      // Applies all necessary corrections to the wcsprm structure
      //  (missing cards, non-standard units or spectral types, ...)
      status = wcsfix(1, (const int*)dim, wcs, stat);
      if(status) {
	std::stringstream errmsg;
	errmsg << "casaImageToWCS: wcsfix failed: Function status returns are:\n";
	for(int i=0; i<NWCSFIX; i++)
	  if (stat[i] > 0) 
	    errmsg << i+1 << ": WCSFIX error code=" << stat[i] << ": "
		   << wcsfix_errmsg[stat[i]] << std::endl;
	ASKAPTHROW(AskapError, errmsg.str());
      }
  
      status=wcsset(wcs);
      if(status)
	ASKAPTHROW(AskapError, "casaImageToWCS: wcsset failed! WCSLIB error code=" << status  <<": "<<wcs_errmsg[status]);


      return wcs;

    }


  }

}
