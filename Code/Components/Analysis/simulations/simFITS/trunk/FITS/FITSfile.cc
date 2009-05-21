/// @file
///
/// Provides base class for handling the creation of FITS files
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <FITS/FITSfile.h>
#include <simulationutilities/SimulationUtilities.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <APS/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <wcslib/wcs.h>
#include <wcslib/wcsunits.h>
#define WCSLIB_GETWCSTAB // define this so that we don't try to redefine wtbarr
                         // (this is a problem when using wcslib-4.2)
#include <fitsio.h>
#include <duchamp/duchamp.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Utils/GaussSmooth.hh>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

ASKAP_LOGGER(logger, ".FITS");

namespace askap
{

  namespace simFITS
  {

    namespace FITS
    {

      FITSfile::FITSfile()
      {
	/// @details Default constructor does not allocate anything, and the arrayAllocated flag is set to false.
	this->itsArrayAllocated = false;
      }

//--------------------------------------------------------

      FITSfile::~FITSfile()
      {
	/// @details Destructor deletes the flux array if it has been allocated.
	if(this->itsArrayAllocated) delete [] this->itsArray;
      }

//--------------------------------------------------------

      FITSfile::FITSfile(const LOFAR::ACC::APS::ParameterSet& parset)
      {
	/// @details Constructor that reads in the necessary
	/// definitions from the parameterset. All FITSfile members
	/// are read in. The conversion factors for the source fluxes
	/// are also defined using the WCSLIB wcsunits function (using
	/// the sourceFluxUnits parameter: if this is not specified,
	/// the fluxes are assumed to be the same units as those of
	/// BUNIT). The pixel array is allocated here.

	ASKAPLOG_DEBUG_STR(logger, "Defining the FITSfile");

	this->itsFileName = parset.getString("filename","");
	this->itsBunit = parset.getString("bunit", "JY/BEAM");

	this->itsSourceList = parset.getString("sourcelist","");
	this->itsPosType = parset.getString("posType","dms");
	std::string sourceFluxUnits = parset.getString("sourceFluxUnits","");
	if(sourceFluxUnits!=""){
	  char *base = (char *)this->itsBunit.c_str();
	  wcsutrn(0,base);
	  char *src = (char *)sourceFluxUnits.c_str();
	  wcsutrn(0,src);
	  int status=wcsunits(src,base,&this->itsUnitScl, &this->itsUnitOff, &this->itsUnitPwr);
	  if(status) ASKAPTHROW(AskapError, "The parameters bunit (\""<<base
				<<"\") and sourceFluxUnits (\"" << src
				<<"\") are not interconvertible.");
	  ASKAPLOG_INFO_STR(logger,"Converting from " << src << " to " << base 
			    << ": " << this->itsUnitScl<< "," <<this->itsUnitOff << "," <<this->itsUnitPwr);
	}
	else {
	  this->itsUnitScl = 1.;
	  this->itsUnitOff = 0.;
	  this->itsUnitPwr = 1.;
	}

	this->itsNoiseRMS = parset.getFloat("noiserms",0.001);

	this->itsDim = parset.getInt32("dim",2);
	this->itsAxes = parset.getInt32Vector("axes");
	if(this->itsAxes.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but axes has " << this->itsAxes.size() << " dimensions.");
	this->itsNumPix = this->itsAxes[0];
	for(uint i=1;i<this->itsDim;i++) this->itsNumPix *= this->itsAxes[i];

	this->itsArray = new float[this->itsNumPix];
	for(int i=0;i<this->itsNumPix;i++) this->itsArray[i] = 0.;
	this->itsArrayAllocated = true;

	this->itsHaveBeam = parset.isDefined("beam");
	if(this->itsHaveBeam) this->itsBeamInfo = parset.getFloatVector("beam");

	this->itsEquinox = parset.getFloat("equinox", 2000.);

	this->itsCTYPE = parset.getStringVector("ctype");
	if(this->itsCTYPE.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but ctype has " << this->itsCTYPE.size() << " dimensions.");
	this->itsCUNIT = parset.getStringVector("cunit");
	if(this->itsCUNIT.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but cunit has " << this->itsCUNIT.size() << " dimensions.");
	this->itsCRVAL = parset.getFloatVector("crval");
	if(this->itsCRVAL.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but crval has " << this->itsCRVAL.size() << " dimensions.");
	this->itsCRPIX = parset.getFloatVector("crpix");
	if(this->itsCRPIX.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but crpix has " << this->itsCRPIX.size() << " dimensions.");
	this->itsCROTA = parset.getFloatVector("crota");
	if(this->itsCROTA.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but crota has " << this->itsCROTA.size() << " dimensions.");
	this->itsCDELT = parset.getFloatVector("cdelt");
	if(this->itsCDELT.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but cdelt has " << this->itsCDELT.size() << " dimensions.");

	this->setWCS();

      }

//--------------------------------------------------------

      void FITSfile::setWCS()
      {

	/// @details The world coordinate system is defined in a
	/// WCSLIB wcsprm struct. This is done by using the CRPIX etc
	/// keyword values.

	ASKAPLOG_DEBUG_STR(logger, "Setting the WCS");
	
	this->itsWCS = (struct wcsprm *)calloc(1,sizeof(struct wcsprm));
	this->itsWCS->flag = -1;
	wcsini(true ,this->itsDim,this->itsWCS);
	this->itsWCS->flag = 0;

	for(uint i=0;i<this->itsDim;i++){
	  this->itsWCS->crpix[i] = this->itsCRPIX[i];
	  this->itsWCS->cdelt[i] = this->itsCDELT[i];
	  this->itsWCS->crval[i] = this->itsCRVAL[i];
	  this->itsWCS->crota[i] = this->itsCROTA[i];
	  strcpy(this->itsWCS->cunit[i], this->itsCUNIT[i].c_str());
	  strcpy(this->itsWCS->ctype[i], this->itsCTYPE[i].c_str());
	}
	this->itsWCS->equinox = this->itsEquinox;

	wcsset(this->itsWCS);

// 	wcsprt(this->itsWCS);

      }

      
//--------------------------------------------------------

      void FITSfile::makeNoiseArray()
      {
	/// @details Fills the pixel array with fluxes sampled from a
	/// normal distribution ~ N(0,itsNoiseRMS) (i.e. the mean of
	/// the distribution is zero). Note that this overwrites the array. 
	
	ASKAPLOG_DEBUG_STR(logger, "Making the noise array");

	for(int i=0;i<this->itsNumPix;i++){
	  this->itsArray[i] = normalRandomVariable(0., this->itsNoiseRMS);
	}

      }

//--------------------------------------------------------

      void FITSfile::addNoise()
      {
	/// @details Adds noise to the array. Noise values are
	/// distributed as N(0,itsNoiseRMS) (i.e. with mean zero).

	ASKAPLOG_DEBUG_STR(logger, "Adding noise");

	for(int i=0;i<this->itsNumPix;i++){
	  this->itsArray[i] += normalRandomVariable(0., this->itsNoiseRMS);
	}

      }

//--------------------------------------------------------

      void FITSfile::addSources()
      {

	/// @details Adds sources to the array. If the source list
	/// file has been defined, it is read one line at a time, and
	/// each source is added to the array. If it is a point source
	/// (i.e. major_axis = 0) then its flux is added to the
	/// relevant pixel, assuming it lies within the boundaries of
	/// the array. If it is a Gaussian source (major_axis>0), then
	/// the function addGaussian is used. The WCSLIB functions are
	/// used to convert the ra/dec positions to pixel positions.

	if(this->itsSourceList.size()>0) { // if the source list is defined.

	  ASKAPLOG_DEBUG_STR(logger, "Adding sources");
	
	  std::ifstream srclist(this->itsSourceList.c_str());
	  std::string ra,dec;
	  double flux,maj,min,pa;
	  double *wld = new double[3];
	  double *pix = new double[3];
	  double *newwld = new double[3];
	  while(srclist >> ra >> dec >> flux >> maj >> min >> pa,
		!srclist.eof()) {

	    // convert fluxes to correct units
	    flux = pow(this->itsUnitScl*flux+this->itsUnitOff, this->itsUnitPwr);

	    // convert sky position to pixels
	    if(this->itsPosType=="dms"){
	      wld[0] = analysis::dmsToDec(ra)*15.;
	      wld[1] = analysis::dmsToDec(dec);
	    }
	    else if(this->itsPosType == "deg"){
	      wld[0] = atof(ra.c_str());
	      wld[1] = atof(dec.c_str());
	    }
	    else ASKAPLOG_ERROR_STR(logger,"Incorrect position type: " << this->itsPosType);
	    wld[2] = 0.;
	    wcsToPixSingle(this->itsWCS,wld,pix);
	    pixToWCSSingle(this->itsWCS,pix,newwld);
	    if(maj>0){
	      // convert widths from arcsec to pixels
	      maj = maj / (3600. * sqrt(fabs(this->itsCDELT[0]*this->itsCDELT[1])));
	      min = min / (3600. * sqrt(fabs(this->itsCDELT[0]*this->itsCDELT[1])));
	      casa::Gaussian2D<casa::Double> gauss(flux,pix[0],pix[1],maj,min/maj,pa);
	      addGaussian(this->itsArray, this->itsAxes, gauss);
	    }
	    else{
	      int loc = int(pix[0]) + this->itsAxes[0]*int(pix[1]);
	      if(pix[0]>=0 && pix[0]<this->itsAxes[0] && pix[1]>=0 && pix[1]<this->itsAxes[1]){
		this->itsArray[loc] += flux;
		ASKAPLOG_DEBUG_STR(logger,"Adding point source of flux " << flux << " to pixel ["<<floor(pix[0])
				   << "," << floor(pix[1]) << "]");
	      }
	    }
	  }
	  delete [] wld;
	  delete [] newwld;
	  delete [] pix;
	  // for each source, add to array

	}

      }


//--------------------------------------------------------

      void FITSfile::convolveWithBeam()
      {

	/// @brief The array is convolved with the Gaussian beam
	/// specified in itsBeamInfo. The GaussSmooth class from the
	/// Duchamp library is used. Note that this is only done if
	/// itsHaveBeam is set true.

	if(!this->itsHaveBeam){
	  ASKAPLOG_WARN_STR(logger, "Cannot convolve with beam as the beam was not specified in the parset.");
	}
	else{

	  ASKAPLOG_DEBUG_STR(logger, "Convolving with the beam");
	  
	  float maj = this->itsBeamInfo[0]/fabs(this->itsCDELT[0]);
	  float min = this->itsBeamInfo[1]/fabs(this->itsCDELT[1]);
	  float pa = this->itsBeamInfo[2];
	  GaussSmooth<float> smoother(maj,min,pa);
	  float *newArray = smoother.smooth(this->itsArray,this->itsAxes[0],this->itsAxes[1]);
	  for(int i=0;i<this->itsNumPix;i++) this->itsArray[i] = newArray[i];
	  delete [] newArray;

	}
      }


//--------------------------------------------------------

      char *numerateKeyword(std::string key, int num)
      {
	/// @details A utility function to combine a keyword and a
	/// value, to produce a relevant FITS keyword for a given
	/// axis. For example numerateKeyword(CRPIX,1) returns CRPIX1.

	std::stringstream ss;
	ss << key << num;
	return (char *)ss.str().c_str();
      }

//--------------------------------------------------------

      void FITSfile::saveFile()
      {

	/// @details Creates a FITS file with the appropriate headers
	/// and saves the flux array into it. Uses the CFITSIO library
	/// to do so.

	ASKAPLOG_DEBUG_STR(logger, "Saving the FITS file");

	int status = 0;
	long *fpixel = new long[this->itsDim];
	for(uint i=0;i<this->itsDim;i++) fpixel[i]=1;
	fitsfile *fptr;         
	if( fits_create_file(&fptr,this->itsFileName.c_str(),&status) ) 
	  fits_report_error(stderr, status);
	
	status = 0;
	long *dim = new long[this->itsDim];
	for(uint i=0;i<this->itsDim;i++) dim[i]=this->itsAxes[i];
	if( fits_create_img(fptr, FLOAT_IMG, this->itsDim, dim, &status) ) 
	  fits_report_error(stderr, status);
	

	status=0; 
	if( fits_update_key(fptr, TFLOAT, "EQUINOX", &(this->itsEquinox), NULL, &status) ) 
	  fits_report_error(stderr, status);
	if(this->itsHaveBeam){
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, "BMAJ", &(this->itsBeamInfo[0]), NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0;
	  if( fits_update_key(fptr, TFLOAT, "BMIN", &(this->itsBeamInfo[1]), NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, "BPA", &(this->itsBeamInfo[2]), NULL, &status) ) 
	    fits_report_error(stderr, status);
	}
	status=0; 
	if( fits_update_key(fptr, TSTRING, "BUNIT", (char *)this->itsBunit.c_str(),  NULL, &status) )
	  fits_report_error(stderr, status);

	for(uint d=0;d<this->itsDim;d++){
	  
	  status=0;
	  if( fits_update_key(fptr, TSTRING, numerateKeyword("CTYPE",d+1), (char *)this->itsCTYPE[d].c_str(),  NULL, &status) ) 
	    fits_report_error(stderr, status);
 	  status=0;
 	  if( fits_update_key(fptr, TSTRING, numerateKeyword("CUNIT",d+1), (char *)this->itsCUNIT[d].c_str(),  NULL, &status) ) 
 	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CRVAL",d+1), &this->itsCRVAL[d], NULL, &status) )
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CDELT",d+1), &this->itsCDELT[d], NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CRPIX",d+1), &this->itsCRPIX[d], NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CROTA",d+1), &this->itsCROTA[d], NULL, &status) ) 
	    fits_report_error(stderr, status);
	}
 
	status = 0;
	if( fits_write_pix(fptr, TFLOAT, fpixel, this->itsNumPix, this->itsArray, &status) )
	  fits_report_error(stderr, status);

	status = 0;
	fits_close_file(fptr, &status);
	if (status){
	  std::cerr << "Error closing file: ";
	  fits_report_error(stderr, status);
	}

      }


    }

  }

}
