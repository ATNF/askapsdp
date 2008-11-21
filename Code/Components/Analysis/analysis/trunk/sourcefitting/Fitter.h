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
      /// @brief Default values for various parameters
      /// @{

      /// @brief Minimum number of pixels that an object has for it to be fit.
      const int defaultMinFitSize = 3;

      /// @brief Width of padding border to put around detections for fitting purposes, in pixels
      const int defaultBoxPadSize = 3;

      /// @brief Default side length of box used to estimate noise for a detection
      const int defaultNoiseBoxSize = 101;

      /// @brief Default maximum number of Gaussian components to fit to a detection
      const int defaultMaxNumFittedGauss = 4;

      /// @brief Default value for the confidence level at which chi-squared values are accepted.
      /// @details If the value is outside the range [0,1], this
      /// method of acceptance is not used and the reduced chi-squared value is
      /// used instead.
      const float defaultChisqConfidence = -1.;

      /// @brief Default value of the maximum permitted reduced chi-squared value for an acceptable fit.
      const float defaultMaxReducedChisq = 5.;

      /// @brief Default value for the maxRMS parameter passed to the casa::fitGaussian::fit() function.
      const Double defaultMaxRMS = 1.;

      /// @}

      /// @ingroup sourcefitting
      /// @brief Simple function to write a list of parameters to the ASKAPLOG
      void logparameters(Matrix<Double> &m);

      /// @ingroup sourcefitting
      /// @brief A class to store parameters that are used by the fitting routines.
      /// @details It stores user-generated parameters, as well as
      ///  things such as detection threshold and peak flux that come from the
      ///  detected object being fitted.
      class FittingParameters
      {
      public:
	/// @brief Default constructor
	FittingParameters(){};
	/// @brief Constructor 
	/// @param parset The parameter set to read parameters from.
	FittingParameters(const LOFAR::ACC::APS::ParameterSet& parset);

	/// @brief Default destructor
	virtual ~FittingParameters(){};
        /// @brief Copy constructor
        FittingParameters(const FittingParameters& f);
        /// @brief Copy function
        FittingParameters& operator= (const FittingParameters& f);


	/// @brief Commands to set and return the various parameters.
	/// @name 
	// @{

	void setMaxNumGauss(int i){itsMaxNumGauss=i;};
	void setBoxPadSize(int i){itsBoxPadSize=i;};
	void setNoiseBoxSize(int i){itsNoiseBoxSize=i;};
	void setMaxRMS(Double d){itsMaxRMS=d;};
	void setChisqConfidence(float f){itsChisqConfidence=f;};
	void setMaxReducedChisq(float f){itsMaxReducedChisq=f;};
	void setPeakFlux(float f){itsSrcPeak = f;};
	void setDetectThresh(float f){itsDetectThresh=f;};
	void setMinFitSize(int i){itsMinFitSize = i;};
	void setBeamSize(float f){itsBeamSize = f;};

	int    maxNumGauss(){return itsMaxNumGauss;};	
	int    boxPadSize(){return itsBoxPadSize;};
	int    noiseBoxSize(){return itsNoiseBoxSize;};
	Double maxRMS(){return itsMaxRMS;};
	float  chisqConfidence(){return itsChisqConfidence;};
	float  maxReducedChisq(){return itsMaxReducedChisq;};
	int    minFitSize(){return itsMinFitSize;};
	float  beamSize(){return itsBeamSize;};

	// @}

	/// @brief Define the box surrounding the detected object
	/// @param box A list of pairs of minimum & maximum values,
	/// one pair for each axis (only x- and y-axes required, as elements 0 and
	/// 1 respectively).
	void saveBox(std::vector<std::pair<long,long> > box){
	  this->itsXmin = box[0].first;
	  this->itsXmax = box[0].second;
	  this->itsYmin = box[1].first;
	  this->itsYmax = box[1].second;
	};
	
	friend class Fitter;

      protected:
	/// @brief The amount of pixels added to the extent of the object to form the box.
	unsigned int itsBoxPadSize;

	/// @brief The maxRMS parameter passed to the casa::FitGaussian::fit() function.
	Double itsMaxRMS;

	/// @brief The maximum number of Gaussian components to be fit.
	unsigned int itsMaxNumGauss;

	/// @brief The confidence level for the chi-squared test. If
	/// outside the [0,1] range, the test is done with the reduced chi-squared
	/// instead.
	float itsChisqConfidence;
	/// @brief The maximum permissible reduced chi-squared value for a fit to be accepted.
	float itsMaxReducedChisq;

	/// @brief The side length of a box centred on the peak pixel used to find the local noise.
	int itsNoiseBoxSize;

	/// @brief The minimum number of pixels an object must have to be fit.
	unsigned int itsMinFitSize;

	/// @brief The flux within the box used for fitting.
	float itsBoxFlux;

	/// @brief The peak flux of the object being fit.
	float itsSrcPeak;

	/// @brief The detection threshold used to obtain the object.
	float itsDetectThresh;

	/// @brief The beam size in the image, using BMIN
	float itsBeamSize;

	/// @brief The extent of the box surrounding the object used for the fitting
	/// @{
	int itsXmin;
	int itsXmax;
	int itsYmin;
	int itsYmax;
	/// @}
      };


      /// @ingroup sourcefitting
      /// @brief A class to manage the 2D profile fitting.
      /// @details The class handles the calling of the fitting
      /// functions, and stores the results using the casa::FitGaussian class
      /// and a casa::Matrix with the best fit. The FittingParameters class
      /// holds the relevant parameters.
      class Fitter
      {
      public:
	/// @brief Default constructor
	Fitter(){};
	/// @brief Default destructor
	virtual ~Fitter(){};
        /// @brief Copy constructor
        Fitter(const Fitter& f);
        /// @brief Copy function
        Fitter& operator= (const Fitter& f);

	/// @brief Set and return the set of fitting parameters
	/// @{
	void setParams(FittingParameters p){itsParams=p;};
	FittingParameters params(){return itsParams;};
	FittingParameters &rparams(){FittingParameters& rfitpars = itsParams; return rfitpars;};
	///@}

	/// @brief Set and return the number of Gaussian components to be fitted.
	/// @{
	void setNumGauss(int i){itsNumGauss=i;};
	int  numGauss(){return itsNumGauss;};
	/// @}

	/// @brief Return the chi-squared value from the fit.
	float chisq(){return itsFitter.chisquared();};
	/// @brief Return the reduced chi-squared value from the fit.
	float redChisq(){return itsRedChisq;};
	/// @brief Return the RMS of the fit
	float RMS(){return itsFitter.RMS();};
	/// @brief Return the number of degrees of freedom of the fit.
	int ndof(){return itsNDoF;};

	/// @brief Set the intial estimates for the Gaussian components.
	void setEstimates(std::vector<SubComponent> &cmpntList, duchamp::FitsHeader &head);
	/// @brief Set the retry factors
	void setRetries();
	/// @brief Set the mask values
	void setMasks();
	/// @brief Fit components to the data
	void fit(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
		 casa::Vector<casa::Double> sigma);

	/// @brief Functions to test the fit according to various criteria.
	/// @{

	/// @brief Has the fit converged?
	bool passConverged();
	/// @brief Does the fit have an acceptable chi-squared value?
	bool passChisq();
	/// @brief Are the fitted components suitably within the box?
	bool passLocation();
	/// @brief Are the component sizes big enough?
	bool passComponentSize();
	/// @brief Are the component fluxes OK?
	bool passComponentFlux();
	bool passPeakFlux();
	bool passIntFlux();
	bool passSeparation();
	/// @brief Is the fit acceptable overall?
	bool acceptable();
	/// @}

	/// @brief Return an ordered list of peak fluxes
	std::multimap<double,int> peakFluxList();

	/// @brief Return a casa::Gaussian2D version of a particular component.
	casa::Gaussian2D<casa::Double> gaussian(int num);

      protected:
	/// @brief The set of parameters defining the fits
	FittingParameters itsParams;

	/// @brief The number of Gaussian functions to fit.
	unsigned int itsNumGauss;
	/// @brief The casa Gaussian Fitter
	FitGaussian<casa::Double> itsFitter;
	/// @brief The number of degrees of freedom in the fit
	int itsNDoF;
	/// @brief The reduced chi-squared of the fit
	float itsRedChisq;

	/// @brief The fitted components
	casa::Matrix<casa::Double> itsSolution;

      };

    }

  }

}

#endif

