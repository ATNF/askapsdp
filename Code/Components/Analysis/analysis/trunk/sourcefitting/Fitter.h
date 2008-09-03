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

#ifndef ASKAP_ANALYSIS_FITTER_H_
#define ASKAP_ANALYSIS_FITTER_H_

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian2D.h>

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

      /// @ingroup sourcefitting
      /// @brief Width of padding border to put around detections for fitting purposes, in pixels
      const int detectionBorder = 3;

      /// @ingroup sourcefitting
      /// @brief Default side length of box used to estimate noise for a detection
      const int defaultNoiseBoxSize = 101;

      /// @ingroup sourcefitting
      const int defaultNumFittedGauss = 4;

      const float defaultChisqCutoff = 0.01;

      const Double defaultMaxRMS = 5.;

      class RadioSource;  // foreshadow RadioSource so that we can make use of it in the following

      class Fitter
      {
      public:
	Fitter(){};
	/// @brief Constructor 
	/// @param parset The parameter set to read Duchamp and other parameters from.
	Fitter(const LOFAR::ACC::APS::ParameterSet& parset);
	virtual ~Fitter(){};

	void setNumGauss(int i){itsNumGauss=i;};
	void setBoxPadSize(int i){itsBoxPadSize=i;};
	void setMaxRMS(Double d){itsMaxRMS=d;};
	void setChisqCutoff(float f){itsChisqCutoff=f;};
	
	float chisq(){return itsFitter.chisquared();};
	float redChisq(){return itsRedChisq;};

	int  numGauss(){return itsNumGauss;};
	int  boxPadSize(){return itsBoxPadSize;};
	Double maxRMS(){return itsMaxRMS;};
	float chisqCutoff(){return itsChisqCutoff;};

	void setEstimates(std::vector<SubComponent> &cmpntList, duchamp::FitsHeader &head);
	void setRetries();
	void setMasks();
	void fit(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
		 casa::Vector<casa::Double> sigma);
	bool acceptable(RadioSource *src);

	std::multimap<double,int> peakFluxList();
	casa::Gaussian2D<casa::Double> gaussian(int num);

      protected:
	unsigned int itsNumGauss;
	unsigned int itsBoxPadSize;
	Double itsMaxRMS;
	float itsChisqCutoff;
	unsigned int itsNoiseBoxSize;

	float itsBoxFlux;
	FitGaussian<casa::Double> itsFitter;
	int itsNDoF;
	float itsRedChisq;

	int itsXmin;
	int itsXmax;
	int itsYmin;
	int itsYmax;

	casa::Matrix<casa::Double> itsSolution;

      };

    }

  }

}

#endif

