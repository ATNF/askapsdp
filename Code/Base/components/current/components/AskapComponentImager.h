/// @file AskapComponentImager.h
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

#ifndef ASKAP_COMPONENTS_ASKAPCOMPONENTIMAGER_H
#define ASKAP_COMPONENTS_ASKAPCOMPONENTIMAGER_H

// ASKAPsoft includes
#include "images/Images/ImageInterface.h"
#include "components/ComponentModels/ComponentList.h"

namespace askap {
namespace components {

/// @brief Project the componentlist onto the image.
/// This class is designed to be interface compatible (in the general sense,
/// not in the OO sense, since there is no interface class) with the casacore
/// Component Imager. This class is based on the implementation of the casacore
/// ComponentImager however is implemented in a manner which should be
/// more performant.
class AskapComponentImager {
    public:
        // Project the componentlist onto the image.
        template<class T>
        static void project(casa::ImageInterface<T>& image, const casa::ComponentList& list);
};

}
}

#endif
