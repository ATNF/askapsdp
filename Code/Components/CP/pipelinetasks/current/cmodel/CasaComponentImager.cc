/// @file CasaComponentImager.cc
///
/// @copyright (c) 2011 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "cmodel/CasaComponentImager.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "skymodelclient/Component.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"
#include "measures/Measures/MDirection.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/GaussianShape.h"
#include "components/ComponentModels/PointShape.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/Flux.h"
#include "images/Images/ComponentImager.h"
#include "images/Images/ImageInterface.h"

// Using
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".CasaComponentImager");

CasaComponentImager::CasaComponentImager(const LOFAR::ParameterSet& parset)
        : itsParset(parset)
{
}

void CasaComponentImager::projectComponents(const std::vector<askap::cp::skymodelservice::Component>& components,
        casa::ImageInterface<casa::Float>& image)
{
    // Build the image
    ComponentImager::project(image, translateComponentList(components));
}

casa::ComponentList CasaComponentImager::translateComponentList(const std::vector<askap::cp::skymodelservice::Component>& components)
{
    casa::ComponentList list;

    std::vector<askap::cp::skymodelservice::Component>::const_iterator it;
    for (it = components.begin(); it != components.end(); ++it) {
        const askap::cp::skymodelservice::Component& c = *it;

        // Build either a GaussianShape or PointShape
        const MDirection dir(c.rightAscension(), c.declination(), MDirection::J2000);
        const Flux<casa::Double> flux(c.i1400().getValue("Jy"), 0.0, 0.0, 0.0);
        const ConstantSpectrum spectrum;

        // Is guassian or point shape?
        if (c.majorAxis().getValue() > 0.0 || c.minorAxis().getValue() > 0.0) {
            ASKAPDEBUGASSERT(c.majorAxis().getValue("arcsec") >= c.minorAxis().getValue("arcsec"));

            // If one is > 0, both must be
            ASKAPDEBUGASSERT(c.majorAxis().getValue() > 0.0);
            ASKAPDEBUGASSERT(c.minorAxis().getValue() > 0.0);

            const GaussianShape shape(dir,
                    c.majorAxis(),
                    c.minorAxis(),
                    c.positionAngle());

            list.add(SkyComponent(flux, shape, spectrum));
        } else {
            const PointShape shape(dir);
            list.add(SkyComponent(flux, shape, spectrum));
        }
    }

    return list;
}
