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

#include <Common/ParameterSet.h>

#include <map>
#include <vector>
#include <utility>


namespace askap {

namespace analysis {

namespace sourcefitting {

/// @ingroup sourcefitting
/// @brief Default values for various parameters
/// @{

/// @brief Minimum number of pixels that an object has for it to be
/// fit.
const unsigned int defaultMinFitSize = 3;

/// @brief Width of padding border to put around detections for
/// fitting purposes, in pixels
const unsigned int defaultBoxPadSize = 3;

/// @brief Default side length of box used to estimate noise for a
/// detection
const unsigned int defaultNoiseBoxSize = 101;

/// @brief Default maximum number of Gaussian components to fit to a
/// detection
const unsigned int defaultMaxNumFittedGauss = 4;

/// @brief Default value for the confidence level at which chi-squared
/// values are accepted.  If the value is outside the range [0,1],
/// this method of acceptance is not used and the reduced chi-squared
/// value is used instead.
const float defaultChisqConfidence = -1.;

/// @brief Default value for the number of thresholds looked at when
/// finding subcomponents
const unsigned int defaultNumSubThresholds = 20;

/// @brief Default value of the maximum permitted reduced chi-squared
/// value for an acceptable fit.
const float defaultMaxReducedChisq = 5.;

/// @brief Default value for the maxRMS parameter passed to the
/// casa::fitGaussian::fit() function.
const Double defaultMaxRMS = 1.;

/// @brief Default value for the maxRetries parameters used by
/// casa::fitGaussian
const unsigned int defaultMaxRetries = 0;

const std::string availableFitTypesArray[5] = {"full", "psf", "shape", "height", "guess"};
const std::vector<std::string>
availableFitTypes(availableFitTypesArray, availableFitTypesArray + 5);

const std::string defaultFitTypesArray[2] = {"full", "psf"};
const std::vector<std::string>
defaultFitTypes(defaultFitTypesArray, defaultFitTypesArray + 2);

/// @}

/// @ingroup sourcefitting
/// @brief Check whether a given fit type is valid
bool isFitTypeValid(std::string type);

/// @ingroup sourcefitting
/// @brief Convert a summary file according to the fit type
/// @details Creates an output file name that indicates
/// the fit type being used. A string "_<type>" is added
/// before any suffix in the base name provided
/// (ie. "_full" or "_psf" or "_shape" or "_height").
/// @param baseName The name of the overall summary file
/// @param type The type of fit being done. If it is not
/// one of "full", "psf", "shape" or "height", the baseName is
/// returned.
/// @return The edited filename.
std::string convertSummaryFile(std::string baseName, std::string type);

/// @ingroup sourcefitting
/// @brief A class to store parameters that are used by the fitting routines.
/// @details It stores user-generated parameters, as well as
///  things such as detection threshold and peak flux that come from the
///  detected object being fitted.
class FittingParameters {
    public:
        /// @brief Default constructor
        FittingParameters() {};
        /// @brief Constructor
        /// @param parset The parameter set to read parameters from.
        FittingParameters(const LOFAR::ParameterSet& parset);

        /// @brief Default destructor
        virtual ~FittingParameters() {};

        /// @brief Functions allowing fitting parameters to be passed over LOFAR Blobs
        /// @name
        /// @{

        /// @brief Pass Fitting parameters into a Blob
        /// @brief This function provides a mechanism for passing the
        /// entire contents of a FittingParameters object into a
        /// LOFAR::BlobOStream stream
        friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream,
                                              FittingParameters& par);

        /// @brief Receive Fitting parameters from a Blob
        /// @brief This function provides a mechanism for receiving the
        /// entire contents of a FittingParameters object from a
        /// LOFAR::BlobIStream stream
        friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream,
                                              FittingParameters& par);

        /// @}

        /// @brief Commands to set and return the various parameters.
        /// @name
        // @{

        void setFlagDoFit(bool b) {itsFlagDoFit = b;};
        void setMaxNumGauss(unsigned int i) {itsMaxNumGauss = i;};
        void setBoxPadSize(unsigned int i) {itsBoxPadSize = i;};
        void setNoiseBoxSize(unsigned int i) {itsNoiseBoxSize = i;};
        void setMaxRMS(Double d) {itsMaxRMS = d;};
        void setChisqConfidence(float f) {itsChisqConfidence = f;};
        void setMaxReducedChisq(float f) {itsMaxReducedChisq = f;};
        void setBoxFlux(float f) {itsBoxFlux = f;};
        void setBoxFlux(casa::Vector<casa::Double> f);
        void setFlagFitJustDetection(bool b) {itsFlagFitJustDetection = b;};
        void setPeakFlux(float f) {itsSrcPeak = f;};
        void setDetectThresh(float f) {itsDetectThresh = f;};
        void setNumSubThresholds(unsigned int i) {itsNumSubThresholds = i;};
        void setFlagLogarithmicIncrements(bool b) {itsFlagLogarithmicIncrements = b;};
        void setFlagUseCurvature(bool b) {itsFlagUseCurvature = b;};
        void setSigmaCurv(float f) {itsSigmaCurv = f;};
        void setCurvatureImage(std::string s) {itsCurvatureImage = s;};
        void setMinFitSize(unsigned int i) {itsMinFitSize = i;};
        void setBeamSize(float f) {itsBeamSize = f;};
        void setMaxRetries(unsigned int i) {itsMaxRetries = i;};
        void setCriterium(Double d) {itsCriterium = d;};
        void setMaxIter(unsigned int i) {itsMaxIter = i;};
        void setNoiseLevel(float f) {itsNoiseLevel = f;};
        void setFlagFitThisParam(unsigned int i, bool b) {itsFlagFitThisParam[i] = b;};

        /// For a given type of fit, set the flags for
        /// each parameter. The types that are possible are:
        /// @li full: All parameters are fitted
        /// @li psf: The shape of the Gaussian is kept fixed,
        /// but the height & location are fitted
        /// @li shape: The height is kept fixed, and the shape
        /// and location are fitted
        /// @li height: The height alone is fitted. All other
        /// parameters, **including position**, are kept fixed.
        /// @param type The string indicating the type of fit. A
        /// warning is given if a different value is provided
        /// and no flags are set.
        void setFlagFitThisParam(std::string type);

        void setStopAfterFirstGoodFit(bool b) {itsStopAfterFirstGoodFit = b;};
        void setUseGuessIfBad(bool b) {itsUseGuessIfBad = b;};
        void setFlagNumGaussFromGuess(bool b) {itsFlagNumGaussFromGuess = b;};
        void setNegativeFluxPossible(bool b) {itsNegativeFluxPossible = b;};
        void setFitTypes(std::vector<std::string> types) {itsFitTypes = types;};

        bool   doFit() {return itsFlagDoFit;};
        unsigned int    maxNumGauss() {return itsMaxNumGauss;};
        unsigned int    boxPadSize() {return itsBoxPadSize;};
        unsigned int    noiseBoxSize() {return itsNoiseBoxSize;};
        Double maxRMS() {return itsMaxRMS;};
        float  chisqConfidence() {return itsChisqConfidence;};
        float  maxReducedChisq() {return itsMaxReducedChisq;};
        float  boxFlux() { return itsBoxFlux;};
        bool   fitJustDetection() {return itsFlagFitJustDetection;};
        float  peakFlux() {return itsSrcPeak;};
        unsigned int   minFitSize() {return itsMinFitSize;};
        unsigned int    numSubThresholds() {return itsNumSubThresholds;};
        bool   flagLogarithmicIncrements() {return itsFlagLogarithmicIncrements;}
        bool   useCurvature() {return itsFlagUseCurvature;}
        float  sigmaCurv() {return itsSigmaCurv;};
        std::string curvatureImage() {return itsCurvatureImage;};
        float  beamSize() {return itsBeamSize;};
        unsigned int    maxRetries() {return itsMaxRetries;};
        Double criterium() {return itsCriterium;};
        unsigned int   maxIter() {return itsMaxIter;};
        bool   useNoise() {return itsUseNoise;};
        float  noiseLevel() {return itsNoiseLevel;};
        bool   stopAfterFirstGoodFit() {return itsStopAfterFirstGoodFit;};
        bool   useGuessIfBad() {return itsUseGuessIfBad;};
        bool   numGaussFromGuess() {return itsFlagNumGaussFromGuess;};
        bool   flagFitThisParam(int i) {return itsFlagFitThisParam[i];};
        bool   negativeFluxPossible() {return itsNegativeFluxPossible;};

        std::vector<std::string> fitTypes() {return itsFitTypes;};
        std::string fitType(unsigned int i) {return itsFitTypes[i];};
        int    numFitTypes() {return itsFitTypes.size();};

        /// @details Whether the given type is one of the fit
        /// types stored in this FittingParameters object.
        /// @param type Fit type under question
        /// @return True if the given type is recorded. False otherwise.
        bool   hasType(std::string type);

        /// @brief Return the number of free parameters
        /// @details Returns the number of parameters that are to be
        /// fitted by the fitting function. This is determined by the
        /// FittingParameters::flagFitThisParam() function, where only
        /// those parameters where the corresponding value of
        /// FittingParameters::itsFlagFitThisParam is true.
        /// @return The number of free parameters in the fit.
        int    numFreeParam();

        // @}

        /// @brief Define the box surrounding the detected object
        /// @param box A list of pairs of minimum & maximum values,
        /// one pair for each axis (only x- and y-axes required, as elements 0 and
        /// 1 respectively).
        void saveBox(casa::Slicer box)
        {
            itsXmin = box.start()[0];
            itsXmax = box.end()[0];
            itsYmin = box.start()[1];
            itsYmax = box.end()[1];
        };

        friend class Fitter;

    protected:

        /// @brief Whether to do a fit
        bool itsFlagDoFit;

        /// @brief The amount of pixels added to the extent of the
        /// object to form the box.
        unsigned int itsBoxPadSize;

        /// @brief The maxRMS parameter passed to the
        /// casa::FitGaussian::fit() function.
        Double itsMaxRMS;

        /// @brief The maximum number of Gaussian components to be
        /// fit.
        unsigned int itsMaxNumGauss;

        /// @brief The confidence level for the chi-squared test. If
        /// outside the [0,1] range, the test is done with the reduced
        /// chi-squared instead.
        float itsChisqConfidence;

        /// @brief The maximum permissible reduced chi-squared value
        /// for a fit to be accepted.
        float itsMaxReducedChisq;

        /// @brief The side length of a box centred on the peak pixel
        /// used to find the local noise.
        unsigned int itsNoiseBoxSize;

        /// @brief The minimum number of pixels an object must have to
        /// be fit.
        unsigned int itsMinFitSize;

        /// @brief The flux within the box used for fitting.
        float itsBoxFlux;

        /// @brief Whether to fit to just the detected pixels (true),
        /// or to use the entire box (false)
        bool  itsFlagFitJustDetection;

        /// @brief The peak flux of the object being fit.
        float itsSrcPeak;

        /// @brief The detection threshold used to obtain the object.
        float itsDetectThresh;

        /// @brief The number of subthresholds used for finding
        /// subcomponents
        unsigned int itsNumSubThresholds;

        /// @brief Whether the subtresholds should be a constant
        /// separation in log space
        bool itsFlagLogarithmicIncrements;

        /// @brief Whether to use a curvature map to estimate initial
        /// guesses
        bool itsFlagUseCurvature;

        /// @brief The measured noise from the curvature map
        float itsSigmaCurv;

        /// @brief The file to which the curvature map is written
        std::string itsCurvatureImage;

        /// @brief Once the initial estimate of components is
        /// determined, only use that number of Gaussians.
        bool itsFlagNumGaussFromGuess;

        /// @brief The beam size in the image, using BMIN
        float itsBeamSize;

        /// @brief The maximum number of retries used by
        /// casa::FitGaussian::fit
        unsigned int itsMaxRetries;

        /// @brief The convergence criterium
        Double itsCriterium;

        /// @brief The maximum number of iterations for
        /// casa::FitGaussian::fit()
        unsigned int itsMaxIter;

        /// @brief Whether to calculate the noise surrounding the
        /// object and use it as the sigma in casa::FitGaussian::fit()
        bool itsUseNoise;

        /// @brief The noise level to use when not calculating it
        float itsNoiseLevel;

        /// @brief Whether the flux of a fitted component can be negative
        bool itsNegativeFluxPossible;

        /// @brief Do we stop after first good fit, or do all fits up
        /// to maxNumGauss?
        bool itsStopAfterFirstGoodFit;

        /// @brief If there is no good fit, should we use the guesses
        /// instead?
        bool itsUseGuessIfBad;

        /// @brief The extent of the box surrounding the object used
        /// for the fitting @{
        int itsXmin;
        int itsXmax;
        int itsYmin;
        int itsYmax;
        /// @}

        /// @brief Flags indicating whether to fit the corresponding
        /// parameter (if true), or whether to leave it untouched
        /// (false)
        std::vector<bool> itsFlagFitThisParam;

        /// List of types of fits to be done: can be:
        /// @li "full" (meaning all parameters are free to be fitted)
        /// @li "psf" (meaning the major/minor axes and the position angle
        /// are kept fixed at the beam size)
        /// @li "shape" (meaning the height is kept fixed, but the shape
        /// is fitted)
        /// @li "height" (meaning the shape AND location are fixed, and
        /// only the height is fitted)
        std::vector<std::string> itsFitTypes;
};

}

}

}

#endif

