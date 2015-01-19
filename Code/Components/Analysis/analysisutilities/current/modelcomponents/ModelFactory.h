/// @file
///
/// Factory way of generating a model component
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
#ifndef ASKAP_ANALYSISUTILS_MODELFACTORY_H_
#define ASKAP_ANALYSISUTILS_MODELFACTORY_H_

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/BeamCorrector.h>
#include <Common/ParameterSet.h>
#include <string>

namespace askap {

namespace analysisutilities {

/// Number of acceptable model types
static const size_t numModelTypes = 11;
/// Set of acceptable model types
const std::string allowedModelTypes[numModelTypes] = {
    "Continuum", "ContinuumID", "Selavy", "POSSUM", "POSSUMHI", "NVSS", "SUMSS",
    "S3SEX", "S3SAX", "Gaussian", "FLASH"
};
/// Return a string listing each possible model type (for use in output)
std::string typeListing();

class ModelFactory {
    public:
        ModelFactory();
        ModelFactory(const LOFAR::ParameterSet& parset);
        ~ModelFactory();

        bool checkType();
        Spectrum* read(std::string line);

    protected:

        std::string itsDatabaseOrigin;
        std::string itsSourceListType;
        float itsBaseFreq;
        float itsRestFreq;
        BeamCorrector itsBeamCorrector;
        bool itsFlagUseDeconvolvedSizes;
        bool itsFlagCorrectForBeam;
};

}
}

#endif
