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

#ifndef ASKAP_ANALYSIS_FITTINGPARAMS_H_
#define ASKAP_ANALYSIS_FITTINGPARAMS_H_

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

      /// @brief Default value for the maxRetries parameters used by casa::fitGaussian
      const int defaultMaxRetries = 0;

      const std::string defaultFitTypesArray[2] = {"full","psf"};
      const std::vector<std::string> defaultFitTypes(defaultFitTypesArray, defaultFitTypesArray+2);

      /// @}

      /// @ingroup sourcefitting
      /// @brief Simple function to write a list of parameters to the ASKAPLOG
      void logparameters(Matrix<Double> &m);

      /// @ingroup sourcefitting
      /// @brief Check whether a given fit type is valid
      bool isFitTypeValid(std::string type);

      /// @ingroup sourcefitting
      /// @brief Convert a summary file according to the fit type
      std::string convertSummaryFile(std::string baseName, std::string type);

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


	/// @brief Functions allowing fitting parameters to be passed over LOFAR Blobs
	/// @name
	/// @{

	/// @brief Pass Fitting parameters into a Blob
	friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream, FittingParameters& par);
	/// @brief Receive Fitting parameters from a Blob
	friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream, FittingParameters& par);
	
	/// @}

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
	void setMinFitSize(unsigned int i){itsMinFitSize = i;};
	void setBeamSize(float f){itsBeamSize = f;};
	void setMaxRetries(int i){itsMaxRetries = i;};
	void setCriterium(Double d){itsCriterium = d;};
	void setMaxIter(uInt i){itsMaxIter = i;};
	void setUseNoise(bool b){itsUseNoise = b;};
	void setFlagFitThisParam(int i, bool b){itsFlagFitThisParam[i] = b;};
	void setFlagFitThisParam(std::string type);

	int    maxNumGauss(){return itsMaxNumGauss;};	
	int    boxPadSize(){return itsBoxPadSize;};
	int    noiseBoxSize(){return itsNoiseBoxSize;};
	Double maxRMS(){return itsMaxRMS;};
	float  chisqConfidence(){return itsChisqConfidence;};
	float  maxReducedChisq(){return itsMaxReducedChisq;};
	unsigned int   minFitSize(){return itsMinFitSize;};
	float  beamSize(){return itsBeamSize;};
	int    maxRetries(){return itsMaxRetries;};
	Double criterium(){return itsCriterium;};
	uInt   maxIter(){return itsMaxIter;};
	bool   useNoise(){return itsUseNoise;};
	bool   flagFitThisParam(int i){return itsFlagFitThisParam[i];};

	std::vector<std::string> fitTypes(){return itsFitTypes;};
	std::string fitType(int i){return itsFitTypes[i];};
	int    numFitTypes(){return itsFitTypes.size();};
	bool   hasType(std::string type);

	/// @brief Return the number of free parameters
	int    numFreeParam();

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

	/// @brief The maximum number of retries used by casa::FitGaussian::fit
	int itsMaxRetries;

	/// @brief The convergence criterium
	Double itsCriterium;

	/// @brief The maximum number of iterations for casa::FitGaussian::fit()
	uInt itsMaxIter;

	/// @brief Whether to calculate the noise surrounding the object and use it as the sigma in casa::FitGaussian::fit()
	bool itsUseNoise;

	/// @brief The extent of the box surrounding the object used for the fitting
	/// @{
	int itsXmin;
	int itsXmax;
	int itsYmin;
	int itsYmax;
	/// @}

	/// @brief Flags indicating whether to fit the corresponding parameter (if true), or whether to leave it untouched (false)
	std::vector<bool> itsFlagFitThisParam;

	/// @brief List of types of fits to be done: can be "full" (meaning all parameters are free to be fitted) or "psf" (meaning the major/minor axes and the position angle are kept fixed at the beam size).
	std::vector<std::string> itsFitTypes;
      };

    }

  }

}

#endif

