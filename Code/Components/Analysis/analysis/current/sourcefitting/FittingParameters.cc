/// @file
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/FittingParameters.h>
#include <sourcefitting/Fitter.h>
#include <sourcefitting/Component.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".sourcefitting");

using namespace duchamp;

namespace askap {

    namespace analysis {

        namespace sourcefitting {

            bool isFitTypeValid(std::string type)
            {
                if (type == "full") return true;

                if (type == "psf")  return true;

                if (type == "shape")  return true;
                if (type == "height")  return true;

                return false;
            }
	  
	  

            FittingParameters::FittingParameters(const LOFAR::ParameterSet& parset)
            {
	      ASKAPLOG_DEBUG_STR(logger, "Parset used by FittingParameters = \n"<<parset);
                this->itsMaxRMS = parset.getDouble("maxRMS", defaultMaxRMS);
                this->itsMaxNumGauss = parset.getInt32("maxNumGauss", defaultMaxNumFittedGauss);
                this->itsBoxPadSize = parset.getInt32("boxPadSize", defaultBoxPadSize);
                this->itsChisqConfidence = parset.getFloat("chisqConfidence", defaultChisqConfidence);
                this->itsMaxReducedChisq = parset.getFloat("maxReducedChisq", defaultMaxReducedChisq);
                this->itsNoiseBoxSize = parset.getInt32("noiseBoxSize", defaultNoiseBoxSize);
                this->itsMinFitSize = parset.getInt32("minFitSize", defaultMinFitSize);
                this->itsNumSubThresholds = parset.getInt32("numSubThresholds", defaultNumSubThresholds);
		this->itsFlagLogarithmicIncrements = parset.getBool("logarithmicThresholds",true);
                this->itsMaxRetries = parset.getInt32("maxRetries", defaultMaxRetries);
                this->itsCriterium = parset.getDouble("criterium", 0.0001);
                this->itsMaxIter = parset.getUint32("maxIter", 1024);
                this->itsUseNoise = parset.getBool("useNoise", true);
		this->itsNoiseLevel = parset.getFloat("noiseLevel",1.);
		this->itsStopAfterFirstGoodFit = parset.getBool("stopAfterFirstGoodFit", false);
		this->itsUseGuessIfBad = parset.getBool("useGuessIfBad",true);
                this->itsFlagFitThisParam = std::vector<bool>(6, true);
		this->itsUseBoxFlux = true;

                if (parset.isDefined("flagFitParam"))
                    ASKAPLOG_WARN_STR(logger, "The flagFitParam parameter is not used any more. Please use fitTypes to specify a list of types of fits.");

                this->itsFitTypes = parset.getStringVector("fitTypes", defaultFitTypes);
                std::stringstream ss;

                std::vector<std::string>::iterator type = this->itsFitTypes.begin();

                while (type < this->itsFitTypes.end()) {
                    if (!isFitTypeValid(*type)) {
                        ASKAPLOG_WARN_STR(logger, "Fit type " << *type << " is not valid. Removing from list.");
                        this->itsFitTypes.erase(type);
                    } else {
                        ss << *type << " ";
                        type++;
                    }
                }

            }

            //**************************************************************//

            FittingParameters::FittingParameters(const FittingParameters& f)
            {
                operator=(f);
            }

            //**************************************************************//

            FittingParameters& FittingParameters::operator= (const FittingParameters& f)
            {
                if (this == &f) return *this;

                this->itsBoxPadSize = f.itsBoxPadSize;
                this->itsMaxRMS = f.itsMaxRMS;
                this->itsMaxNumGauss = f.itsMaxNumGauss;
                this->itsChisqConfidence = f.itsChisqConfidence;
                this->itsMaxReducedChisq = f.itsMaxReducedChisq;
                this->itsNoiseBoxSize = f.itsNoiseBoxSize;
                this->itsMinFitSize = f.itsMinFitSize;
                this->itsBoxFlux = f.itsBoxFlux;
		this->itsUseBoxFlux = f.itsUseBoxFlux;
                this->itsXmin = f.itsXmin;
                this->itsYmin = f.itsYmin;
                this->itsXmax = f.itsXmax;
                this->itsYmax = f.itsYmax;
                this->itsSrcPeak = f.itsSrcPeak;
                this->itsDetectThresh = f.itsDetectThresh;
                this->itsNumSubThresholds = f.itsNumSubThresholds;
		this->itsFlagLogarithmicIncrements = f.itsFlagLogarithmicIncrements;
                this->itsBeamSize = f.itsBeamSize;
                this->itsMaxRetries = f.itsMaxRetries;
                this->itsCriterium = f.itsCriterium;
                this->itsMaxIter = f.itsMaxIter;
                this->itsUseNoise = f.itsUseNoise;
		this->itsNoiseLevel = f.itsNoiseLevel;
		this->itsStopAfterFirstGoodFit = f.itsStopAfterFirstGoodFit;
		this->itsUseGuessIfBad = f.itsUseGuessIfBad;
                this->itsFlagFitThisParam = f.itsFlagFitThisParam;
                this->itsFitTypes = f.itsFitTypes;
                return *this;
            }

            //**************************************************************//

	    void FittingParameters::setBoxFlux(casa::Vector<casa::Double> f)
	    {
	      
	      this->itsBoxFlux = 0.;
	      for (uint i = 0; i < f.size(); i++) 
		this->itsBoxFlux += f(i);

	    } 

            //**************************************************************//

            void FittingParameters::setFlagFitThisParam(std::string type)
            {
                /// @details For a given type of fit, set the flags for
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

                if (type == "full") {
                    for (int i = 0; i < 6; i++) this->itsFlagFitThisParam[i] = true;
                } else if (type == "psf") {
                    for (int i = 0; i < 3; i++) this->itsFlagFitThisParam[i] = true;

                    for (int i = 3; i < 6; i++) this->itsFlagFitThisParam[i] = false;
                } else if (type == "shape") {
                    this->itsFlagFitThisParam[0] = false;

                    for (int i = 1; i < 6; i++) this->itsFlagFitThisParam[i] = true;

                } else if (type == "height") {
                    this->itsFlagFitThisParam[0] = true;

                    for (int i = 1; i < 6; i++) this->itsFlagFitThisParam[i] = false;

                } else {
                    ASKAPLOG_WARN_STR(logger, "Fit type " << type << " is not valid, so can't set parameter flags");
                }
            }

            //**************************************************************//

            int FittingParameters::numFreeParam()
            {
                /// @details Returns the number of parameters that are to be
                /// fitted by the fitting function. This is determined by the
                /// FittingParameters::flagFitThisParam() function, where only
                /// those parameters where the corresponding value of
                /// FittingParameters::itsFlagFitThisParam is true.
                /// @return The number of free parameters in the fit.
                int nparam = 0;

                for (unsigned int p = 0; p < 6; p++)
                    if (this->itsFlagFitThisParam[p]) nparam++;

                return nparam;
            }

            //**************************************************************//

            bool FittingParameters::hasType(std::string type)
            {
                /// @details Whether the given type is one of the fit
                /// types stored in this FittingParameters object.
                /// @param type Fit type under question
                /// @return True if the given type is recorded. False otherwise.

                bool haveIt = false;

                for (unsigned int i = 0; i < this->itsFitTypes.size() && !haveIt; i++) {
                    haveIt = (this->itsFitTypes[i] == type);
                }

                return haveIt;
            }

            //**************************************************************//

            std::string convertSummaryFile(std::string baseName, std::string type)
            {
                /// @details Creates an output file name that indicates
                /// the fit type being used. A string "_<type>" is added
                /// before any suffix in the base name provided
                /// (ie. "_full" or "_psf" or "_shape" or "_height").
                /// @param baseName The name of the overall summary file
                /// @param type The type of fit being done. If it is not
                /// one of "full", "psf", "shape" or "height", the baseName is
                /// returned.
                /// @return The edited filename.

                std::string suffix;
                size_t loc = baseName.rfind(".");
                suffix = baseName.substr(loc, baseName.length());
                std::string outName = baseName;

                if (type == "full") outName.replace(loc, suffix.length(), "_full" + suffix);
                else if (type == "psf") outName.replace(loc, suffix.length(), "_psf" + suffix);
                else if (type == "shape") outName.replace(loc, suffix.length(), "_shape" + suffix);
                else if (type == "height") outName.replace(loc, suffix.length(), "_height" + suffix);

                return outName;
            }


	  
        }

    }

}
