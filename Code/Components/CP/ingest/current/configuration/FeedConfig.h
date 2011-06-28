/// @file FeedConfig.h
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

#ifndef ASKAP_CP_CPINGEST_FEEDCONFIG_H
#define ASKAP_CP_CPINGEST_FEEDCONFIG_H

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief TODO: Write documentation...
class FeedConfig {
    public:

        /// @brief Constructor
        /// @param[in] offsets  Feeds (or synthesised beam) offsets in radians. The
        ///                     Matrix is sized Matrix(nFeeds,2). I.E. An offset in
        ///                     X and in Y for each feed. The first column is the
        ///                     offset in X and the second the offset in Y.
        ///
        /// @param[in] pols    Polarisations, size if nFeeds.
        FeedConfig(const casa::Matrix<casa::Quantity>& offsets,
                   const casa::Vector<casa::String>& pols);

        casa::uInt nFeeds(void) const;

        casa::Quantity offsetX(casa::uInt i) const;

        casa::Quantity offsetY(casa::uInt i) const;

        casa::String pol(casa::uInt i) const;

    private:
        casa::Matrix<casa::Quantity> itsOffsets;
        casa::Vector<casa::String> itsPols;
};

}
}
}

#endif
