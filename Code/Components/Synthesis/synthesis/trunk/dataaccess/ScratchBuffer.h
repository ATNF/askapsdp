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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef SCRATCH_BUFFER_H
#define SCRATCH_BUFFER_H

#include <casa/Arrays/Cube.h>
#include <casa/BasicSL/Complex.h>

namespace conrad {

namespace synthesis {

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

} // namespace synthesis

} // namespace conrad

#endif // #ifndef SCRATCH_BUFFER_H
