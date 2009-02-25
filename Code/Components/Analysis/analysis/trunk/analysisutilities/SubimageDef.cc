/// @file
///
/// @brief Define and access subimages of a FITS file.
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/CasaImageUtil.h>

#include <gsl/gsl_sf_gamma.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageOpener.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
                         //  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

using namespace casa;

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap
{
  namespace analysis
  {

    SubimageDef::SubimageDef()
    {
      this->itsNAxis = 0; 
      this->itsNSubX = 1;
      this->itsNSubY = 1;
      this->itsNSubZ = 1;
      this->itsOverlapX = this->itsOverlapY = this->itsOverlapZ = 0;
      this->itsImageName = "";
    }
    
    SubimageDef::~SubimageDef()
    {
      if(this->itsNAxis > 0){
	delete [] this->itsNSub;
	delete [] this->itsOverlap;
      }
    }

    SubimageDef::SubimageDef(const LOFAR::ACC::APS::ParameterSet& parset)
    {
      this->itsNAxis = 0;
      this->itsImageName = parset.getString("image");
      this->itsNSubX = parset.getInt16("nsubx",1);
      this->itsNSubY = parset.getInt16("nsuby",1);
      this->itsNSubZ = parset.getInt16("nsubz",1);
      this->itsOverlapX = parset.getInt16("overlapx",0);
      this->itsOverlapY = parset.getInt16("overlapy",0);
      this->itsOverlapZ = parset.getInt16("overlapz",0);  
    }


    SubimageDef& SubimageDef::operator= (const SubimageDef& s)
    {
      /// @details Copy constructor for SubimageDef, that does a deep
      /// copy of the itsNSub and itsOverlap arrays.

      if(this==&s) return *this;
      this->itsNSubX = s.itsNSubX;
      this->itsNSubY = s.itsNSubY;
      this->itsNSubZ = s.itsNSubZ;
      this->itsOverlapX = s.itsOverlapX;
      this->itsOverlapY = s.itsOverlapY;
      this->itsOverlapZ = s.itsOverlapZ;
      this->itsNAxis = s.itsNAxis;
      this->itsFullImageDim = s.itsFullImageDim;
      this->itsImageName = s.itsImageName;
      if(this->itsNAxis > 0){
	for(int i=0;i<this->itsNAxis;i++){
	  this->itsNSub[i] = s.itsNSub[i];
	  this->itsOverlap[i] = s.itsOverlap[i];
	}
      }
      return *this;
    }

    void SubimageDef::define(wcsprm *wcs)
    {

      /// @details Define all the necessary variables within the
      /// SubimageDef class. The image (given by the parameter "image"
      /// in the parset) is to be split up according to the nsubx/y/z
      /// parameters, with overlaps in each direction given by the
      /// overlapx/y/z parameters (these are in pixels).
      ///
      /// The WCS parameters in wcs determine which axes are the x, y and z
      /// axes. The number of axes is also determined from the WCS
      /// parameter set.
      ///
      /// @param wcs The WCSLIB definition of the world coordinate system

      this->itsNAxis = wcs->naxis;
      const int lng  = wcs->lng;
      const int lat  = wcs->lat;
      const int spec = wcs->spec;
      
//       ASKAPLOG_DEBUG_STR(logger, "SubimageDef::define : " << this->itsNAxis << " axes, " << lng << " " << lat << " " << spec);

      if(this->itsNAxis>0){
	this->itsNSub = new int[this->itsNAxis];
	this->itsOverlap = new int[this->itsNAxis];
	for(int i=0;i<this->itsNAxis;i++){
	  if(i==lng){ 
	    this->itsNSub[i]=this->itsNSubX; 
	    this->itsOverlap[i]=this->itsOverlapX; 
	  }
	  else if(i==lat){
	    this->itsNSub[i]=this->itsNSubY; 
	    this->itsOverlap[i]=this->itsOverlapY;
	  }
	  else if(i==spec){
	    this->itsNSub[i]=this->itsNSubZ;
	    this->itsOverlap[i]=this->itsOverlapZ; 
	  }
	  else{
	    this->itsNSub[i] = 1; 
	    this->itsOverlap[i] = 0;
	  }
	}
      }

//       for(int i=0;i<this->itsNAxis;i++){
// 	ASKAPLOG_DEBUG_STR(logger, "SubimageDef::define : axis#" << i << ": " << this->itsNSub[i] << " " << this->itsOverlap[i]);
//       }
    }

    void SubimageDef::defineFITS(std::string FITSfilename)
    {
      /// @details Define all the necessary variables within the
      /// SubimageDef class. The image (given by the parameter "image"
      /// in the parset) is to be split up according to the nsubx/y/z
      /// parameters, with overlaps in each direction given by the
      /// overlapx/y/z parameters (these are in pixels).
      ///
      /// This version is designed for FITS files. The Duchamp
      /// function duchamp::FitsHeader::defineWCS() is used to extract
      /// the WCS parameters from the FITS header. This is then sent
      /// to SubimageDef::define(wcsprm *) to define everything.
      ///
      /// @param FITSfilename The name of the FITS file.

      duchamp::Param tempPar; // This is needed for defineWCS(), but we don't care about the contents.
      duchamp::FitsHeader imageHeader;
      this->itsImageName = FITSfilename;
      imageHeader.defineWCS(this->itsImageName,tempPar);
      this->define(imageHeader.getWCS());
    }

    duchamp::Section SubimageDef::section(int workerNum, std::string inputSubsection)
    {

      /// @details Return the subsection object for the given worker
      /// number. (These start at 0). The subimages are tiled across
      /// the cube with the x-direction varying quickest, then y, then
      /// z. The dimensions of the array are obtained with the
      /// getFITSdimensions(std::string) function.
      /// @return A duchamp::Section object containing all information
      /// on the subsection.

      ASKAPLOG_INFO_STR(logger, "Input subsection to be used is " << inputSubsection);
      duchamp::Section inputSec(inputSubsection);
      inputSec.parse(this->itsFullImageDim);
      ASKAPLOG_INFO_STR(logger, "Input subsection is OK");

//       long start = 0;
      
      long sub[3];
      sub[0] = workerNum % this->itsNSub[0];
      sub[1] = (workerNum % (this->itsNSub[0]*this->itsNSub[1])) / this->itsNSub[0];
      sub[2] = workerNum / (this->itsNSub[0]*this->itsNSub[1]);

      std::stringstream section;

      int numAxes = this->itsFullImageDim.size();

      for(int i=0;i<numAxes;i++){
	    
	if(this->itsNSub[i] > 1){
// 	  int min = std::max( start, sub[i]*(this->itsFullImageDim[i]/this->itsNSub[i]) - this->itsOverlap[i]/2 ) + 1;
// 	  int max = std::min( this->itsFullImageDim[i], (sub[i]+1)*(this->itsFullImageDim[i]/this->itsNSub[i]) + this->itsOverlap[i]/2 );
	  int length = inputSec.getDim(i);
	  float sublength = float(length)/float(this->itsNSub[i]);

// 	  ASKAPLOG_DEBUG_STR(logger, "SubimageDef::section : axis#" << i << " full length = " << this->itsFullImageDim[i]
// 			     << ", length = " << length << ", inputSection = " << inputSec.getSection(i) <<", sublength = " << sublength);
// 	  ASKAPLOG_DEBUG_STR(logger, "SubimageDef::section : input min = " << inputSec.getStart(i) << ", input max = " << inputSec.getEnd(i));
// 	  ASKAPLOG_DEBUG_STR(logger, "SubimageDef::section : sub min = " << inputSec.getStart(i) + sub[i]*(length/this->itsNSub[i]) - this->itsOverlap[i]/2
// 			     << ", sub max = " << inputSec.getStart(i) + (sub[i]+1)*(length/this->itsNSub[i]) + this->itsOverlap[i]/2 );

	  int min = std::max( long(inputSec.getStart(i)), long(inputSec.getStart(i) + sub[i]*sublength - this->itsOverlap[i]/2) ) + 1;
	  int max = std::min( long(inputSec.getEnd(i)+1), long(inputSec.getStart(i) + (sub[i]+1)*sublength + this->itsOverlap[i]/2) );

// 	  ASKAPLOG_DEBUG_STR(logger, "SubimageDef::section : min = " << min << ", max = " << max);
	  section << min << ":" << max;
	}
	else //section << "*";
	  section << inputSec.getSection(i);
// 	if(i != this->itsNAxis-1) section << ",";
	if(i != numAxes-1) section << ",";
	  
      }
      std::string secstring = "["+section.str()+"]";
      ASKAPLOG_INFO_STR(logger, "New subsection to be used is " << secstring);
      duchamp::Section sec(secstring);
      sec.parse(this->itsFullImageDim);

      return sec;
    }


  }

}
