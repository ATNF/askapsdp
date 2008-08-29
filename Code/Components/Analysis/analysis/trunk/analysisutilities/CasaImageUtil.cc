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

#include <duchamp/duchamp.hh>
#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>

using namespace casa;
using namespace duchamp;

namespace askap
{

  namespace analysis
  {

    void getCasaImage(std::string imageName)
    {

      wcsprm *wcs = casaImageToWCS(imageName);

      duchamp::FitsHeader head;
      duchamp::Param par;
      
      storeWCStoHeader(head,par,wcs);

      

    }

    void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs)
    {
      if(wcs->spec>=0){ //if there is a spectral axis

	int index = wcs->spec;
	std::string desiredType,specType = wcs->ctype[index];
	std::string shortType = specType.substr(0,4);
	if(shortType=="VELO" || shortType=="VOPT" || shortType=="ZOPT" 
	   || shortType=="VRAD" || shortType=="BETA"){
	  if(wcs->restfrq != 0){
	    // Set the spectral axis to a standard specification: VELO-F2V
	    desiredType = duchampVelocityType;
	    if(wcs->restwav == 0) 
	      wcs->restwav = 299792458.0 /  wcs->restfrq;
	    head.setSpectralDescription(duchampSpectralDescription[VELOCITY]);
	  }
	  else{
	    // No rest frequency defined, so put spectral dimension in frequency. 
	    // Set the spectral axis to a standard specification: FREQ
	    duchampWarning("Cube Reader",
			   "No rest frequency defined. Using frequency units in spectral axis.\n");
	    desiredType = duchampFrequencyType;
	    par.setSpectralUnits("MHz");
	    if(strcmp(wcs->cunit[index],"")==0){
	      duchampWarning("Cube Reader",
			     "No frequency unit given. Assuming frequency axis is in Hz.\n");
	      strcpy(wcs->cunit[index],"Hz");
	    }
	    head.setSpectralDescription(duchampSpectralDescription[FREQUENCY]);
	  }
	}
	else {
	  desiredType = duchampFrequencyType;
	  par.setSpectralUnits("MHz");
	  if(strcmp(wcs->cunit[index],"")==0){
	    duchampWarning("Cube Reader",
			   "No frequency unit given. Assuming frequency axis is in Hz.\n");
	    strcpy(wcs->cunit[index],"Hz");
	  }
	  head.setSpectralDescription(duchampSpectralDescription[FREQUENCY]);
	}	

	// Now we need to make sure the spectral axis has the correct setup.
	//  We use wcssptr to translate it if it is not of the desired type,
	//  or if the spectral units are not defined.

	bool needToTranslate = false;

	//       if(strncmp(specType.c_str(),desiredType.c_str(),4)!=0) 
	// 	needToTranslate = true;

	std::string blankstring = "";
	if(strcmp(wcs->cunit[wcs->spec],blankstring.c_str())==0)
	  needToTranslate = true;

	if(needToTranslate){

	  if(strcmp(wcs->ctype[wcs->spec],"VELO")==0)
	    strcpy(wcs->ctype[wcs->spec],"VELO-F2V");

	  index = wcs->spec;
	
	  int status = wcssptr(wcs, &index, (char *)desiredType.c_str());
	  if(status){
	    std::stringstream errmsg;
	    errmsg<< "WCSSPTR failed! Code=" << status << ": "
		  << wcs_errmsg[status] << std::endl
		  << "(wanted to convert from type \"" << specType
		  << "\" to type \"" << desiredType << "\")\n";
	    duchampWarning("Cube Reader",errmsg.str());

	  }

	}
    
      } // end of if(wcs->spec>=0)

	// Save the wcs to the FitsHeader class that is running this function
      head.setWCS(wcs);
      head.setNWCS(1);
      

    }

    wcsprm *casaImageToWCS(std::string imageName)
    {
      LatticeBase* lattPtr = ImageOpener::openImage (imageName);
      ImageInterface<Float>* imagePtr = dynamic_cast<ImageInterface<Float>*>(lattPtr);

      return casaImageToWCS(imagePtr);
    }

    wcsprm *casaImageToWCS(ImageInterface<Float>* imagePtr)
    {

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
