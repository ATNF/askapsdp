/// @file ImageFactory.h
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

#ifndef ASKAP_CP_PIPELINETASKS_IMAGEFACTORY_H
#define ASKAP_CP_PIPELINETASKS_IMAGEFACTORY_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Arrays/Vector.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "images/Images/TempImage.h"
#include "images/Images/PagedImage.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

class ImageFactory {
    public:
        static casa::TempImage<casa::Float> createTempImage(const LOFAR::ParameterSet& parset);

        static casa::PagedImage<casa::Float> createPagedImage(const LOFAR::ParameterSet& parset,
                const std::string& filename);

    private:
        // Create a coordinate system
        // @note The image parameters are read from the parset
        static casa::CoordinateSystem createCoordinateSystem(casa::uInt nx, casa::uInt ny,
                const LOFAR::ParameterSet& parset);

        // Convert a std::vector of strings (either I, Q, U or V) to a vector of
        // integers mapping to casa::Stokes types
        static casa::Vector<casa::Int> parseStokes(const std::vector<std::string>& input);

        // Given a coordinate system instance, return the number of stokes parameters
        static casa::uInt getNumStokes(const casa::CoordinateSystem& coordsys);
};

}
}
}

#endif
