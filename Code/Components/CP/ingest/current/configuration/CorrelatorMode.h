/// @file CorrelatorMode.h
///
/// @copyright (c) 2014 CSIRO
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
#include <string>
#include <vector>

// ASKAPsoft includes
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/Stokes.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class encapsulates a "scan", a part of a larger observation.
class CorrelatorMode {
    public:

        /// @brief Constructor
        CorrelatorMode(const std::string& modeName,
             const casa::Quantity& chanWidth,
             const casa::uInt nChan,
             const std::vector<casa::Stokes::StokesTypes>& stokes,
             const casa::uInt interval);

        /// @brief Returns the correlator mode name
        std::string name(void) const;

        /// @brief The number of spectral channels
        casa::uInt nChan(void) const;

        /// @brief The width (in Hz) of a single spectral channel.
        /// @note This may be a negative width in the case where increasing
        /// channel number corresponds to decreasing frequency.
        casa::Quantity chanWidth(void) const;

        /// @brief The stokes types to be observed
        std::vector<casa::Stokes::StokesTypes> stokes(void) const;

        /// @brief Returns, in microseconds, correlator integration interval.
        casa::uInt interval(void) const;

    private:
        std::string itsModeName;
        casa::Quantity itsChanWidth;
        casa::uInt itsNChan;
        std::vector<casa::Stokes::StokesTypes> itsStokes;
        casa::uInt itsInterval;
};

}
}
}

#endif
