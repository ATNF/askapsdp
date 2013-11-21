/// @file IFrtApproach.h
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ASKAP_CP_INGEST_IFRTAPPROACH_H
#define ASKAP_CP_INGEST_IFRTAPPROACH_H

// ASKAPsoft includes
#include "cpcommon/VisChunk.h"

// casa includes
#include <casa/Arrays/Matrix.h>

namespace askap {
namespace cp {
namespace ingest {

/// @brief interface for an fringe rotation method
/// @details A number of different approaches to fringe rotation are possible (i.e. with/without DRx, with/without hw-rotator
/// with more or with less correction in the software. It seems convenient to represent all different approaches by a hierarchy of
/// classes and get the task itself responsible for just delay and rate calculation (as accurate as possible, approximations and caching 
/// are done in implementations of this interface)
class IFrtApproach {
    public:
        /// @brief destructor to keep the compiler happy, nothing is done
        virtual ~IFrtApproach();

        /// Process a VisChunk.
        ///
        /// This method is called once for each correlator integration.
        /// 
        /// @param[in] chunk    a shared pointer to a VisChunk object. The
        ///             VisChunk contains all the visibilities and associated
        ///             metadata for a single correlator integration. This method
        ///             is expected to correct visibilities in this VisChunk 
        ///             as required (some methods may not need to do any correction at all)
        /// @param[in] delays matrix with delays for all antennas (rows) and beams (columns) in seconds
        /// @param[in] rates matrix with phase rates for all antennas (rows) and 
        ///                  beams (columns) in radians per second
        virtual void process(const askap::cp::common::VisChunk::ShPtr& chunk, 
                     const casa::Matrix<double> &delays, const casa::Matrix<double> &rates) = 0;

        /// Shared pointer definition
        typedef boost::shared_ptr<IFrtApproach> ShPtr;
};

} // namespace ingest 
} // namespace cp 
} // namespace askap 

#endif // #ifndef ASKAP_CP_INGEST_IFRTAPPROACH_H

