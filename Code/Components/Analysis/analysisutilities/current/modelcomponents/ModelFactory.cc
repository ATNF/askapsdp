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
    this->itsDatabaseOrigin = parset.getString("database", "Continuum");
    if (!this->checkType())
        ASKAPLOG_ERROR_STR(logger, "Database type '" << this->itsDatabaseOrigin <<
                           "' is not valid.");
    this->itsSourceListType = parset.getString("sourcelisttype", "continuum");
    this->itsBaseFreq = parset.getFloat("baseFreq", 1400.);
    this->itsRestFreq = parset.getFloat("restFreq", nu0_HI);
    this->itsFlagUseDeconvolvedSizes = parset.getBool("useDeconvolvedSizes", false);
    this->itsFlagCorrectForBeam = parset.getBool("correctForBeam", false) &&
                                  !this->itsFlagUseDeconvolvedSizes;
    if (this->itsFlagCorrectForBeam) {
        LOFAR::ParameterSet subset = parset.makeSubset("correctForBeam.");
        this->itsBeamCorrector = BeamCorrector(subset);
    }
}

ModelFactory::~ModelFactory()
{
}

bool ModelFactory::checkType()
{
    bool isOK = false;
    for (size_t i = 0; i < numModelTypes; i++) {
        isOK = isOK || (this->itsDatabaseOrigin == allowedModelTypes[i]);
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
            ASKAPTHROW(AskapError, "'this->itsDatabase' parameter has incompatible value '"
                       << this->itsDatabaseOrigin
                       << "' - needs to be one of: " << typeListing());
        } else if (this->itsDatabaseOrigin == "Continuum") {
            boost::shared_ptr<Continuum> cont(new Continuum);
            cont->setNuZero(this->itsBaseFreq);
            cont->define(line);
            src = boost::shared_ptr<Spectrum>(cont.get());
        } else if (this->itsDatabaseOrigin == "ContinuumID") {
            boost::shared_ptr<ContinuumID> cont(new ContinuumID);
            cont->setNuZero(this->itsBaseFreq);
            cont->define(line);
            src = boost::shared_ptr<Spectrum>(cont.get());
        } else if (this->itsDatabaseOrigin == "Selavy") {
            boost::shared_ptr<ContinuumSelavy>
            sel(new ContinuumSelavy(this->itsFlagUseDeconvolvedSizes));
            sel->setNuZero(this->itsBaseFreq);
            sel->define(line);
            src = boost::shared_ptr<Spectrum>(sel.get());
        } else if (this->itsDatabaseOrigin == "POSSUM") {
            boost::shared_ptr<FullStokesContinuum> stokes(new FullStokesContinuum);
            stokes->setNuZero(this->itsBaseFreq);
            stokes->define(line);
            src = boost::shared_ptr<Spectrum>(stokes.get());
        } else if (this->itsDatabaseOrigin == "POSSUMHI") {
            boost::shared_ptr<FullStokesContinuumHI> stokesHI(new FullStokesContinuumHI);
            stokesHI->setNuZero(this->itsBaseFreq);
            stokesHI->define(line);
            src = boost::shared_ptr<Spectrum>(stokesHI.get());
        } else if (this->itsDatabaseOrigin == "NVSS") {
            boost::shared_ptr<ContinuumNVSS> nvss(new ContinuumNVSS);
            nvss->setNuZero(this->itsBaseFreq);
            nvss->define(line);
            src = boost::shared_ptr<Spectrum>(nvss.get());
        } else if (this->itsDatabaseOrigin == "SUMSS") {
            boost::shared_ptr<ContinuumSUMSS> sumss(new ContinuumSUMSS);
            sumss->setNuZero(this->itsBaseFreq);
            sumss->define(line);
            src = boost::shared_ptr<Spectrum>(sumss.get());
        } else if (this->itsDatabaseOrigin == "S3SEX") {
            if (this->itsSourceListType == "continuum") {
                boost::shared_ptr<ContinuumS3SEX> contS3SEX(new ContinuumS3SEX);
                contS3SEX->setNuZero(this->itsBaseFreq);
                contS3SEX->define(line);
                src = boost::shared_ptr<Spectrum>(contS3SEX.get());
            } else if (this->itsSourceListType == "spectralline") {
                boost::shared_ptr<HIprofileS3SEX> profSEX(new HIprofileS3SEX);
                profSEX->define(line);
                src = boost::shared_ptr<Spectrum>(profSEX.get());
            }
        } else if (this->itsDatabaseOrigin == "S3SAX") {
            boost::shared_ptr<HIprofileS3SAX> profSAX(new HIprofileS3SAX);
            profSAX->define(line);
            src = boost::shared_ptr<Spectrum>(profSAX.get());
        } else if (this->itsDatabaseOrigin == "Gaussian") {
            boost::shared_ptr<GaussianProfile> profGauss(new GaussianProfile(this->itsRestFreq));
            profGauss->define(line);
            src = boost::shared_ptr<Spectrum>(profGauss.get());
        } else if (this->itsDatabaseOrigin == "FLASH") {
            boost::shared_ptr<FLASHProfile> profFLASH(new FLASHProfile(this->itsRestFreq));
            profFLASH->define(line);
            src = boost::shared_ptr<Spectrum>(profFLASH.get());
        }
    }

    if (this->itsFlagCorrectForBeam)
        this->itsBeamCorrector.convertSource(src);

    return src;

}

}
}
