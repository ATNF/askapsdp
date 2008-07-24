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
#include <simutils/simUtils.h>
#include <evaluationutilities/EvaluationUtilities.h>

#include <APS/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <wcslib/wcs.h>
#define WCSLIB_GETWCSTAB // define this so that we don't try to redefine wtbarr
                         // (this is a problem when using wcslib-4.2)
#include <fitsio.h>
#include <duchamp/Utils/utils.hh>

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

  namespace simulations
  {

    namespace FITS
    {

      FITSfile::FITSfile(const LOFAR::ACC::APS::ParameterSet& parset)
      {

	this->itsFileName = parset.getString("filename","");

	this->itsSourceList = parset.getString("sourcelist","");
	this->itsNoiseRMS = parset.getFloat("noiserms",0.001);

	this->itsDim = parset.getInt32("dim",2);
	this->itsAxes = parset.getInt32Vector("axes");
	if(this->itsAxes.size() != this->itsDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
		     << ", but axes has " << this->itsAxes.size() << " dimensions.");
	this->itsNumPix = this->itsAxes[0];
	for(int i=1;i<this->itsNumPix;i++) this->itsNumPix *= this->itsAxes[i];

	this->itsBeamInfo = parset.getFloatVector("beam");

	this->itsEquinox = parset.getFloat("equinox", 2000.);
	this->itsBunit = parset.getString("bunit", "JY/BEAM");

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

      void FITSfile::setWCS()
      {

	wcsini(1,this->itsDim,this->itsWCS);
	this->itsWCS->flag = 0;

	for(int i=0;i<this->itsDim;i++){
	  this->itsWCS->crpix[i] = this->itsCRPIX[i];
	  this->itsWCS->cdelt[i] = this->itsCDELT[i];
	  this->itsWCS->crval[i] = this->itsCRVAL[i];
	  this->itsWCS->crota[i] = this->itsCROTA[i];
	  strcpy(this->itsWCS->cunit[i], this->itsCUNIT[i].c_str());
	  strcpy(this->itsWCS->ctype[i], this->itsCTYPE[i].c_str());
	}
	this->itsWCS->equinox = this->itsEquinox;

      }

      
      void FITSfile::makeNoiseArray()
      {

	if(this->itsArrayAllocated) delete [] this->itsArray;

	this->itsArray = new float[this->itsNumPix];

	for(int i=0;i<this->itsNumPix;i++)
	  this->itsArray[i] = normalRandomVariable(0., this->itsNoiseRMS);

      }


      void FITSfile::addSources()
      {
	
	// read in source list --> vector of Gaussian2Ds

	std::ifstream srclist(this->itsSourceList.c_str());
	std::string ra,dec;
	double flux,maj,min,pa;
	double wld[2],pix[2];
	std::vector<casa::Gaussian2D<casa::Double> > sources;
	while(srclist >> ra >> dec >> flux >> maj >> min >> pa,
	      !srclist.eof()) {
	  wld[0] = evaluation::dmsToDec(ra)*15.;
	  wld[1] = evaluation::dmsToDec(dec);
	  wcsToPixSingle(this->itsWCS,wld,pix);
	  casa::Gaussian2D<casa::Double> gauss(pix[0],pix[1],flux,maj,min,pa);
	  sources.push_back(gauss);
	}

	// for each source, add to array

	for(int i=0;i<sources.size();i++) 
	  addGaussian(this->itsArray, sources[i]);

      }


      char *numerateKeyword(std::string key, int num)
      {
	std::stringstream ss;
	ss << key << num;
	return (char *)ss.str().c_str();
      }

      void FITSfile::saveFile()
      {

	int status = 0,bitpix;
	long *fpixel = new long[2];
	for(int i=0;i<2;i++) fpixel[i]=1;
	fitsfile *fptr;         
	if( fits_create_file(&fptr,this->itsFileName.c_str(),&status) ) 
	  fits_report_error(stderr, status);
	
	status = 0;
	long *dim = new long[this->itsDim];
	for(int i=0;i<this->itsDim;i++) dim[i]=this->itsAxes[i];
	if( fits_create_img(fptr, FLOAT_IMG, this->itsNumPix, dim, &status) ) 
	  fits_report_error(stderr, status);
	
	status = 0;
	if( fits_write_pix(fptr, TFLOAT, fpixel, this->itsNumPix, this->itsArray, &status) )
	  fits_report_error(stderr, status);


	status=0; 
	if( fits_update_key(fptr, TFLOAT, "EQUINOX", &(this->itsEquinox), NULL, &status) ) 
	  fits_report_error(stderr, status);
	status=0; 
	if( fits_update_key(fptr, TFLOAT, "BMAJ", &(this->itsBeamInfo[0]), NULL, &status) ) 
	  fits_report_error(stderr, status);
	status=0;
	if( fits_update_key(fptr, TFLOAT, "BMIN", &(this->itsBeamInfo[1]), NULL, &status) ) 
	  fits_report_error(stderr, status);
	status=0; 
	if( fits_update_key(fptr, TFLOAT, "BPA", &(this->itsBeamInfo[2]), NULL, &status) ) 
	  fits_report_error(stderr, status);
	status=0; 
	if( fits_update_key(fptr, TSTRING, "BUNIT", (char *)this->itsBunit.c_str(),  NULL, &status) )
	  fits_report_error(stderr, status);

	for(int d=0;d<this->itsDim;d++){
	  
	  status=0;
	  if( fits_update_key(fptr, TSTRING, numerateKeyword("CTYPE1",d), (char *)this->itsCTYPE[d].c_str(),  NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CRVAL1",d), &this->itsCRVAL[d], NULL, &status) )
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CDELT1",d), &this->itsCDELT[d], NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CRPIX1",d), &this->itsCRPIX[d], NULL, &status) ) 
	    fits_report_error(stderr, status);
	  status=0; 
	  if( fits_update_key(fptr, TFLOAT, numerateKeyword("CROTA1",d), &this->itsCROTA[d], NULL, &status) ) 
	    fits_report_error(stderr, status);
	}
 
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
