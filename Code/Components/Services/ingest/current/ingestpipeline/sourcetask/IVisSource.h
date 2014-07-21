/// @file IVisSource.h
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

#ifndef ASKAP_CP_INGEST_IVISSOURCE_H
#define ASKAP_CP_INGEST_IVISSOURCE_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "cpcommon/VisDatagram.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief An interface for a class that can be used as a source of VisDatagram
/// objects.
class IVisSource {
    public:
        /// @brief Destructor.
        virtual ~IVisSource() {};

        /// @brief Returns the next VisDatagram object.
        ///
        /// @param[in] timeout how long to wait for data before returning
        ///         a null pointer, in the case where the
        ///         buffer is empty. The timeout is in microseconds,
        ///         and anything less than zero will result in no
        ///         timeout (i.e. blocking functionality).
        ///
        /// @return a shared pointer to a VisDatagram object.
        virtual boost::shared_ptr<VisDatagram> next(const long timeout = -1) = 0;

        // Shared pointer definition
        typedef boost::shared_ptr<IVisSource> ShPtr;
};

}
}
}

#endif
