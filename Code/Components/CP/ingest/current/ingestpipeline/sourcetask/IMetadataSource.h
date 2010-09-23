/// @file IMetadataSource.h
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_INGEST_IMETADATASOURCE_H
#define ASKAP_CP_INGEST_IMETADATASOURCE_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "cpcommon/TosMetadata.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief An interface for a class that can be used as a source of TosMetadata
/// objects.
class IMetadataSource {
    public:
        virtual ~IMetadataSource() {};

        /// @brief Returns the next TosMetadata object.
        /// This call is blocking, it will not return until an object is
        /// available to return.
        ///
        /// @return a shared pointer to a TosMetadata object.
        virtual boost::shared_ptr<askap::cp::TosMetadata> next(void) = 0;

        // Shared pointer definition
        typedef boost::shared_ptr<IMetadataSource> ShPtr;
};

}
}
}

#endif
