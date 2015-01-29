/// @file
///
/// Generates a model component from an input, for a given model type
///
/// @copyright (c) 2010 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

#include <askap_analysisutilities.h>

#include <modelcomponents/ModelFactory.h>
#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumID.h>
#include <modelcomponents/ContinuumS3SEX.h>
#include <modelcomponents/ContinuumSelavy.h>
#include <modelcomponents/ContinuumNVSS.h>
#include <modelcomponents/ContinuumSUMSS.h>
#include <modelcomponents/GaussianProfile.h>
#include <modelcomponents/FLASHProfile.h>
#include <modelcomponents/HIprofileS3SEX.h>
#include <modelcomponents/HIprofileS3SAX.h>
#include <modelcomponents/FullStokesContinuum.h>
#include <modelcomponents/FullStokesContinuumHI.h>
#include <modelcomponents/BeamCorrector.h>
#include <coordutils/SpectralUtilities.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".modelfactory");

namespace askap {

namespace analysisutilities {

ModelFactory::ModelFactory()
{
}

ModelFactory::ModelFactory(const LOFAR::ParameterSet& parset)
{
    itsDatabaseOrigin = parset.getString("database", "Continuum");
    if (!this->checkType()) {
        ASKAPLOG_ERROR_STR(logger, "Database type '" << itsDatabaseOrigin << "' is not valid.");
    }
    itsSourceListType = parset.getString("sourcelisttype", "continuum");
    itsBaseFreq = parset.getFloat("baseFreq", 1400.);
    itsRestFreq = parset.getFloat("restFreq", nu0_HI);
    itsFlagUseDeconvolvedSizes = parset.getBool("useDeconvolvedSizes", false);
    itsFlagCorrectForBeam = parset.getBool("correctForBeam", false) &&
                            !itsFlagUseDeconvolvedSizes;
    if (itsFlagCorrectForBeam) {
        LOFAR::ParameterSet subset = parset.makeSubset("correctForBeam.");
        itsBeamCorrector = BeamCorrector(subset);
    }
}

ModelFactory::~ModelFactory()
{
}

bool ModelFactory::checkType()
{
    bool isOK = false;
    for (size_t i = 0; i < numModelTypes; i++) {
        isOK = isOK || (itsDatabaseOrigin == allowedModelTypes[i]);
    }
    return isOK;
}

std::string typeListing()
{
    std::string listing = "";
    for (size_t i = 0; i < numModelTypes; i++) {
        listing = listing + "'" + allowedModelTypes[i] + "'";
        if (i != (numModelTypes - 1)) {
            listing = listing + " , ";
        }
    }
    return listing;
}

boost::shared_ptr<Spectrum> ModelFactory::read(std::string line)
{
    boost::shared_ptr<Spectrum>src;

    if (line[0] != '#') {  // ignore commented lines

        if (!this->checkType()) {
            ASKAPTHROW(AskapError, "'itsDatabase' parameter has incompatible value '"
                       << itsDatabaseOrigin << "' - needs to be one of: " << typeListing());
        } else if (itsDatabaseOrigin == "Continuum") {
            src.reset(new Continuum(line, itsBaseFreq));
        } else if (itsDatabaseOrigin == "ContinuumID") {
            src.reset(new ContinuumID(line, itsBaseFreq));
        } else if (itsDatabaseOrigin == "Selavy") {
            src.reset(new ContinuumSelavy(line, itsBaseFreq, itsFlagUseDeconvolvedSizes));
        } else if (itsDatabaseOrigin == "POSSUM") {
            src.reset(new FullStokesContinuum(line, itsBaseFreq));
        } else if (itsDatabaseOrigin == "POSSUMHI") {
            src.reset(new FullStokesContinuumHI(line, itsBaseFreq));
        } else if (itsDatabaseOrigin == "NVSS") {
            src.reset(new ContinuumNVSS(line, itsBaseFreq));
        } else if (itsDatabaseOrigin == "SUMSS") {
            src.reset(new ContinuumSUMSS(line, itsBaseFreq));
        } else if (itsDatabaseOrigin == "S3SEX") {
            if (itsSourceListType == "continuum") {
                src.reset(new ContinuumS3SEX(line, itsBaseFreq));
            } else if (itsSourceListType == "spectralline") {
                src.reset(new HIprofileS3SEX(line));
            }
        } else if (itsDatabaseOrigin == "S3SAX") {
            src.reset(new HIprofileS3SAX(line));
        } else if (itsDatabaseOrigin == "Gaussian") {
            src.reset(new GaussianProfile(line, itsRestFreq));
        } else if (itsDatabaseOrigin == "FLASH") {
            src.reset(new FLASHProfile(line, itsRestFreq));
        }
    }

    if (itsFlagCorrectForBeam)
        itsBeamCorrector.convertSource(src);

    return src;

}

}
}
