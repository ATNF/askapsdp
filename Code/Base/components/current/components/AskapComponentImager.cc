/// @file AskapComponentImager.cc
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
#include "AskapComponentImager.h"

// Include package level header file
#include "askap_components.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "images/Images/ImageInterface.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/Flux.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/SpectralIndex.h"
#include "components/ComponentModels/SpectralModel.h"
#include "components/ComponentModels/ComponentType.h"
#include "components/ComponentModels/ComponentShape.h"
#include "components/ComponentModels/PointShape.h"
#include "components/ComponentModels/GaussianShape.h"
#include "components/ComponentModels/DiskShape.h"

ASKAP_LOGGER(logger, ".AskapComponentImager");

using namespace askap;
using namespace askap::components;
using namespace casa;

template<class T>
void project(casa::ImageInterface<T>& image, const casa::ComponentList& list)
{
    if (list.nelements() == 0) {
        return;
    }

    const CoordinateSystem& coords = image.coordinates();
    const IPosition imageShape = image.shape();


    // Process each SkyComponent individually
    for (uInt i = 0; i < list.nelements(); ++i) {
        const SkyComponent& c = list.component(i);
    }
}
