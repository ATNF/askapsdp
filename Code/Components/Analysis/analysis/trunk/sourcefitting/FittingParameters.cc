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

                return false;
            }

            FittingParameters::FittingParameters(const LOFAR::ACC::APS::ParameterSet& parset)
            {
                this->itsMaxRMS = parset.getDouble("maxRMS", defaultMaxRMS);
                this->itsMaxNumGauss = parset.getInt32("maxNumGauss", defaultMaxNumFittedGauss);
                this->itsBoxPadSize = parset.getInt32("boxPadSize", defaultBoxPadSize);
                this->itsChisqConfidence = parset.getFloat("chisqConfidence", defaultChisqConfidence);
                this->itsMaxReducedChisq = parset.getFloat("maxReducedChisq", defaultMaxReducedChisq);
                this->itsNoiseBoxSize = parset.getInt32("noiseBoxSize", defaultNoiseBoxSize);
                this->itsMinFitSize = parset.getInt32("minFitSize", defaultMinFitSize);
                this->itsMaxRetries = parset.getInt32("maxRetries", defaultMaxRetries);
                this->itsCriterium = parset.getDouble("criterium", 0.0001);
                this->itsMaxIter = parset.getUint32("maxIter", 1024);
                this->itsUseNoise = parset.getBool("useNoise", true);
                //  this->itsFlagFitThisParam = parset.getBoolVector("flagFitParam",std::vector<bool>(6,true));
                this->itsFlagFitThisParam = std::vector<bool>(6, true);

                if (parset.isDefined("flagFitParam"))
                    ASKAPLOG_WARN_STR(logger, "The flagFitParam parameter is not used any more. Please use fitTypes to specify a list of types of fits.");

//  std::vector<std::string> defaultFitTypes;
//  defaultFitTypes.push_back("full");
//  defaultFitTypes.push_back("psf");
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

                ASKAPLOG_DEBUG_STR(logger, "Fit types being used: " << ss.str());
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
                this->itsXmin = f.itsXmin;
                this->itsYmin = f.itsYmin;
                this->itsXmax = f.itsXmax;
                this->itsYmax = f.itsYmax;
                this->itsSrcPeak = f.itsSrcPeak;
                this->itsDetectThresh = f.itsDetectThresh;
                this->itsBeamSize = f.itsBeamSize;
                this->itsMaxRetries = f.itsMaxRetries;
                this->itsCriterium = f.itsCriterium;
                this->itsMaxIter = f.itsMaxIter;
                this->itsUseNoise = f.itsUseNoise;
                this->itsFlagFitThisParam = f.itsFlagFitThisParam;
                this->itsFitTypes = f.itsFitTypes;
                return *this;
            }

            //**************************************************************//

            void FittingParameters::setFlagFitThisParam(std::string type)
            {
                if (type == "full") {
                    for (int i = 0; i < 6; i++) this->itsFlagFitThisParam[i] = true;
                } else if (type == "psf") {
                    for (int i = 0; i < 3; i++) this->itsFlagFitThisParam[i] = true;

                    for (int i = 3; i < 6; i++) this->itsFlagFitThisParam[i] = false;
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
                bool haveIt = false;

                for (unsigned int i = 0; i < this->itsFitTypes.size() && !haveIt; i++) {
                    haveIt = (this->itsFitTypes[i] == type);
                }

                return haveIt;
            }

            //**************************************************************//

            std::string convertSummaryFile(std::string baseName, std::string type)
            {
                std::string suffix;
                size_t loc = baseName.rfind(".");
                suffix = baseName.substr(loc, baseName.length());
                std::string outName = baseName;

                if (type == "full") outName.replace(loc, suffix.length(), "_full" + suffix);
                else if (type == "psf") outName.replace(loc, suffix.length(), "_psf" + suffix);

                return outName;
            }


        }

    }

}
