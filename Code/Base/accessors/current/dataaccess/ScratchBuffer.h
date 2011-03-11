/// @file
///
/// @brief Helper class representing a scratch buffer
/// @details Read-write access idiom is to work with so-called
/// buffers, a chunk of visibility data sharing the same metadada
/// with the main accessor (see IDataAccessor for more info).
/// This is a helper class used between ITableDataIterator and
/// ITableDataAccessor, which represents one scratch buffer used to
/// cache disk information for each current iteration.
/// 
/// @copyright (c) 2007 CSIRO
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
///

#ifndef SCRATCH_BUFFER_H
#define SCRATCH_BUFFER_H

#include <casa/Arrays/Cube.h>
#include <casa/BasicSL/Complex.h>

namespace askap {

namespace accessors {

/// @brief Helper class representing a scratch buffer
/// @details Read-write access idiom is to work with so-called
/// buffers, a chunk of visibility data sharing the same metadada
/// with the main accessor (see IDataAccessor for more info).
/// This is a helper class used between ITableDataIterator and
/// ITableDataAccessor, which represents one scratch buffer used to
/// cache disk information for each current iteration.
/// @note an std::pair can be used instead, but this class gives
/// a better looking code
/// @ingroup dataaccess_hlp
struct ScratchBuffer {
    ScratchBuffer() : needsRead(true), needsFlush(false) {}
    
    /// visibility cube
    casa::Cube<casa::Complex> vis;
    /// True if the cube has to be initialized (reading is required)
    mutable bool needsRead;
    /// True if there was a write operation and the cube has to be
    /// flushed back to disk (or whatever is used for storage of buffers)
    mutable bool needsFlush;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef SCRATCH_BUFFER_H
