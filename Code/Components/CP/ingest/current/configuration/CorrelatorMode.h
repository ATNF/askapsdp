/// @file CorrelatorMode.h
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

#ifndef ASKAP_CP_INGEST_CORRELATORMODE_H
#define ASKAP_CP_INGEST_CORRELATORMODE_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "measures/Measures/Stokes.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief TODO: Write documentation...
class CorrelatorMode {
    public:

        /// @brief Constructor
        CorrelatorMode(const casa::String& name,
                       const casa::uInt nChan,
                       const casa::Quantity chanWidth,
                       const std::vector<casa::Stokes::StokesTypes> stokes);

        casa::String name(void) const;
        casa::uInt nChan(void) const;
        casa::Quantity chanWidth(void) const;
        std::vector<casa::Stokes::StokesTypes> stokes(void) const;

    private:

        casa::String itsName;
        casa::uInt itsNChan;
        casa::Quantity itsChanWidth;
        std::vector<casa::Stokes::StokesTypes> itsStokes;

};

}
}
}

#endif
