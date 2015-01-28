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

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

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
    //          ASKAPLOG_DEBUG_STR(logger, "Parset used by FittingParameters = \n"<<parset);
    itsFlagDoFit = parset.getBool("doFit", false);
    itsMaxRMS = parset.getDouble("maxRMS", defaultMaxRMS);
    itsMaxNumGauss = parset.getInt32("maxNumGauss", defaultMaxNumFittedGauss);
    itsBoxPadSize = parset.getInt32("boxPadSize", defaultBoxPadSize);
    itsChisqConfidence = parset.getFloat("chisqConfidence", defaultChisqConfidence);
    itsMaxReducedChisq = parset.getFloat("maxReducedChisq", defaultMaxReducedChisq);
    itsNoiseBoxSize = parset.getInt32("noiseBoxSize", defaultNoiseBoxSize);
    itsMinFitSize = parset.getInt32("minFitSize", defaultMinFitSize);
    itsNumSubThresholds = parset.getInt32("numSubThresholds", defaultNumSubThresholds);
    itsFlagLogarithmicIncrements = parset.getBool("logarithmicThresholds", true);
    itsFlagUseCurvature = parset.getBool("useCurvature", false);
    itsCurvatureImage = parset.getString("curvatureImage", "");
    itsMaxRetries = parset.getInt32("maxRetries", defaultMaxRetries);
    itsCriterium = parset.getDouble("criterium", 0.0001);
    itsMaxIter = parset.getUint32("maxIter", 1024);
    itsUseNoise = parset.getBool("useNoise", true);
    itsNoiseLevel = parset.getFloat("noiseLevel", 1.);
    itsStopAfterFirstGoodFit = parset.getBool("stopAfterFirstGoodFit", true);
    itsFlagNumGaussFromGuess = parset.getBool("numGaussFromGuess", true);
    itsUseGuessIfBad = parset.getBool("useGuessIfBad", true);
    itsFlagFitThisParam = std::vector<bool>(6, true);
    itsFlagFitJustDetection = parset.getBool("fitJustDetection", true);

    if (parset.isDefined("flagFitParam"))
        ASKAPLOG_WARN_STR(logger, "The flagFitParam parameter is not used any more. " <<
                          "Please use fitTypes to specify a list of types of fits.");

    itsFitTypes = parset.getStringVector("fitTypes", defaultFitTypes);
    std::stringstream ss;

    std::vector<std::string>::iterator type = itsFitTypes.begin();

    while (type < itsFitTypes.end()) {
        if (!isFitTypeValid(*type)) {
            ASKAPLOG_ERROR_STR(logger, "Fit type " << *type <<
                               " is not valid. Removing from list.");
            itsFitTypes.erase(type);
        } else {
            ss << *type << " ";
            type++;
        }
    }

    if (itsFlagDoFit && itsFitTypes.size() == 0) {
        itsFlagDoFit = false;
    }

    if (itsFlagUseCurvature && 
        itsCurvatureImage == ""){
        ASKAPLOG_ERROR_STR(logger, 
                           "No curvature image has been set via the curvatureImage parameter. " <<
                           " Setting useCurvature=false.");
        itsFlagUseCurvature = false;
    }

}

//**************************************************************//

void FittingParameters::setBoxFlux(casa::Vector<casa::Double> f)
{

    itsBoxFlux = 0.;
    for (uint i = 0; i < f.size(); i++)
        itsBoxFlux += f(i);

}

//**************************************************************//

void FittingParameters::setFlagFitThisParam(std::string type)
{

    if (type == "full") {
        for (int i = 0; i < 6; i++) {
            itsFlagFitThisParam[i] = true;
        }
    } else if (type == "psf") {
        for (int i = 0; i < 3; i++) {
            itsFlagFitThisParam[i] = true;
        }
        for (int i = 3; i < 6; i++) {
            itsFlagFitThisParam[i] = false;
        }
    } else if (type == "shape") {
        itsFlagFitThisParam[0] = false;
        for (int i = 1; i < 6; i++) {
            itsFlagFitThisParam[i] = true;
        }
    } else if (type == "height") {
        itsFlagFitThisParam[0] = true;
        for (int i = 1; i < 6; i++) {
            itsFlagFitThisParam[i] = false;
        }
    } else {
        ASKAPLOG_WARN_STR(logger, "Fit type " << type <<
                          " is not valid, so can't set parameter flags");
    }
}

//**************************************************************//

int FittingParameters::numFreeParam()
{
    int nparam = 0;

    for (unsigned int p = 0; p < 6; p++) {
        if (itsFlagFitThisParam[p]) nparam++;
    }

    return nparam;
}

//**************************************************************//

bool FittingParameters::hasType(std::string type)
{

    bool haveIt = false;

    for (unsigned int i = 0; i < itsFitTypes.size() && !haveIt; i++) {
        haveIt = (itsFitTypes[i] == type);
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

    if (type == "full") {
        outName.replace(loc, suffix.length(), "_full" + suffix);
    } else if (type == "psf") {
        outName.replace(loc, suffix.length(), "_psf" + suffix);
    } else if (type == "shape") {
        outName.replace(loc, suffix.length(), "_shape" + suffix);
    } else if (type == "height") {
        outName.replace(loc, suffix.length(), "_height" + suffix);
    } else {
        ASKAPLOG_WARN_STR(logger, "Fit type " << type <<
                          " is not valid, so can't set parameter flags");
    }
    return outName;
}

//**************************************************************//

LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &blob, FittingParameters& par)
{
    blob << par.itsFlagDoFit;
    blob << par.itsBoxPadSize;
    blob << par.itsMaxRMS;
    blob << par.itsMaxNumGauss;
    blob << par.itsChisqConfidence;
    blob << par.itsMaxReducedChisq;
    blob << par.itsNoiseBoxSize;
    blob << par.itsMinFitSize;
    blob << par.itsBoxFlux;
    blob << par.itsFlagFitJustDetection;
    blob << par.itsSrcPeak;
    blob << par.itsDetectThresh;
    blob << par.itsNumSubThresholds;
    blob << par.itsFlagLogarithmicIncrements;
    blob << par.itsFlagUseCurvature;
    blob << par.itsSigmaCurv;
    blob << par.itsCurvatureImage;
    blob << par.itsFlagNumGaussFromGuess;
    blob << par.itsBeamSize;
    blob << par.itsMaxRetries;
    blob << par.itsCriterium;
    blob << par.itsMaxIter;
    blob << par.itsUseNoise;
    blob << par.itsNoiseLevel;
    blob << par.itsNegativeFluxPossible;
    blob << par.itsStopAfterFirstGoodFit;
    blob << par.itsUseGuessIfBad;
    blob << par.itsXmin;
    blob << par.itsXmax;
    blob << par.itsYmin;
    blob << par.itsYmax;
    uint32 size = par.itsFlagFitThisParam.size();
    blob << size;
    for (uint32 i = 0; i < size; i++) {
        blob << par.itsFlagFitThisParam[i];
    }
    size = par.itsFitTypes.size();
    blob << size;
    for (uint32 i = 0; i < size; i++) {
        blob << par.itsFitTypes[i];
    }

    return blob;
}

//**************************************************************//

LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &blob, FittingParameters& par)
{
    blob >> par.itsFlagDoFit;
    blob >> par.itsBoxPadSize;
    blob >> par.itsMaxRMS;
    blob >> par.itsMaxNumGauss;
    blob >> par.itsChisqConfidence;
    blob >> par.itsMaxReducedChisq;
    blob >> par.itsNoiseBoxSize;
    blob >> par.itsMinFitSize;
    blob >> par.itsBoxFlux;
    blob >> par.itsFlagFitJustDetection;
    blob >> par.itsSrcPeak;
    blob >> par.itsDetectThresh;
    blob >> par.itsNumSubThresholds;
    blob >> par.itsFlagLogarithmicIncrements;
    blob >> par.itsFlagUseCurvature;
    blob >> par.itsSigmaCurv;
    blob >> par.itsCurvatureImage;
    blob >> par.itsFlagNumGaussFromGuess;
    blob >> par.itsBeamSize;
    blob >> par.itsMaxRetries;
    blob >> par.itsCriterium;
    blob >> par.itsMaxIter;
    blob >> par.itsUseNoise;
    blob >> par.itsNoiseLevel;
    blob >> par.itsNegativeFluxPossible;
    blob >> par.itsStopAfterFirstGoodFit;
    blob >> par.itsUseGuessIfBad;
    blob >> par.itsXmin;
    blob >> par.itsXmax;
    blob >> par.itsYmin;
    blob >> par.itsYmax;
    int32 size;
    bool flag;
    blob >> size;
    par.itsFlagFitThisParam = std::vector<bool>(size);
    for (int i = 0; i < size; i++) {
        blob >> flag;
        par.itsFlagFitThisParam[i] = flag;
    }
    blob >> size;
    par.itsFitTypes = std::vector<std::string>(size);
    std::string type;
    for (int i = 0; i < size; i++) {
        blob >> type;
        par.itsFitTypes[i] = type;
    }

    return blob;
}




}

}

}
