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
#ifndef ASKAP_SIMFITS_FITS_H_
#define ASKAP_SIMFITS_FITS_H_

#include <wcslib/wcs.h>

#include <APS/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap
{

  namespace simFITS
  {

    namespace FITS
    {

      class FITSfile
      {
      public:
	FITSfile(){
	  itsArrayAllocated = false;
	};

	virtual ~FITSfile(){
	  if(itsArrayAllocated) delete [] itsArray;
	};
	
	/// @brief Constructor, using an input parameter set
	FITSfile(const LOFAR::ACC::APS::ParameterSet& parset);

	void setWCS();

	void makeNoiseArray();

	void addNoise();

	void addSources();

	void convolveWithBeam();

	void saveFile();

//	void setDim(int d){itsDim=d;};
//	void dim(){return itsDim;};
//	void setAxes(std::vector<int> a){itsAxes = a;};
//	//	void setAxes(int i,{itsAxes = a;};

      protected:

	std::string itsFileName;
	std::string itsSourceList;

	float *itsArray;
	bool itsArrayAllocated;
	float itsNoiseRMS;
	
	unsigned int itsDim;
	std::vector<int> itsAxes;
	int itsNumPix;
	
	bool itsHaveBeam;
	std::vector<float> itsBeamInfo;
	
	float itsEquinox;
	std::string itsBunit;
       
	double itsUnitScl;
	double itsUnitOff;
	double itsUnitPwr;

	struct wcsprm *itsWCS;

	std::vector<std::string> itsCTYPE;
	std::vector<std::string> itsCUNIT;
	std::vector<float> itsCRVAL;
	std::vector<float> itsCRPIX;
	std::vector<float> itsCROTA;
	std::vector<float> itsCDELT;


      };

    }

  }
}

#endif
