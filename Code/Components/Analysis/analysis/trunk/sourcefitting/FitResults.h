/// @file
///
/// Contains the calls to the fitting routines, as well as parameters such as number of Gaussians and box widths
///
/// @copyright (c) 2008 CSIRO
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

#ifndef ASKAP_ANALYSIS_FITRESULTS_H_
#define ASKAP_ANALYSIS_FITRESULTS_H_

#include <sourcefitting/Fitter.h>
#include <sourcefitting/Component.h>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian2D.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <duchamp/fitsHeader.hh>

#include <casa/namespace.h>

#include <APS/ParameterSet.h>

#include <map>
#include <vector>
#include <utility>


namespace askap
{

  namespace analysis
  {

    namespace sourcefitting
    {



      class FitResults
      {
      public:
	FitResults(){}
	virtual ~FitResults(){};
        /// @brief Copy constructor
        FitResults(const FitResults& f);
        /// @brief Copy function
        FitResults& operator= (const FitResults& f);

	void saveResults(Fitter &fit);

	float chisq(){return itsChisq;};
	float redchisq(){return itsRedChisq;};
	float RMS(){return itsRMS;};
	int   ndof(){return itsNumDegOfFreedom;};
	int   numFreeParam(){return itsNumFreeParam;};
	int   numGauss(){return itsNumGauss;};
	std::vector<casa::Gaussian2D<double> > fitSet(){return itsGaussFitSet;};
	/// @brief Return a reference to the set of Gaussian fits.
	std::vector<casa::Gaussian2D<Double> >& fits(){
	  std::vector<casa::Gaussian2D<Double> >& rfit = itsGaussFitSet; return rfit;};
	
	int numFits(){return itsGaussFitSet.size();};

	/// @brief Functions allowing fitting parameters to be passed over LOFAR Blobs
	/// @name
	/// @{

	/// @brief Pass Fitting parameters into a Blob
	friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream, FitResults& par);
	/// @brief Receive Fitting parameters from a Blob
	friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream, FitResults& par);
	
	/// @}


      protected:
	float itsChisq;
	float itsRedChisq;
	float itsRMS;
	int itsNumDegOfFreedom;
	int itsNumFreeParam;
	int itsNumGauss;
	/// @brief A two-dimensional Gaussian fit to the object.
	std::vector<casa::Gaussian2D<Double> > itsGaussFitSet;

      };


    }

  }

}

#endif

