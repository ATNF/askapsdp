/// @file
/// @brief an implementation of IDataAccessor for buffers
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. It deals with
/// writable buffers only. Another class TableDataAccessor is
/// intended to write to the original visibility data.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/TableBufferDataAccessor.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataIterator.h>

using namespace conrad;
using namespace conrad::synthesis;

/// construct an object linked with the given const accessor and
/// non-const iterator (which provides a read/write functionality)
/// @param name a name of the buffer represented by this accessor
/// @param iter a reference to the associated read-write iterator
TableBufferDataAccessor::TableBufferDataAccessor(const std::string &name,
                                   const TableDataIterator &iter) :
                 MetaDataAccessor(iter.getAccessor()), itsName(name),
			     itsIterator(iter) {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableBufferDataAccessor::visibility() const
{
  // active buffer should be returned
  fillBufferIfNeeded();
  return itsScratchBuffer.vis;  
}

/// read the information into the buffer if necessary
void TableBufferDataAccessor::fillBufferIfNeeded() const
{
  if (itsScratchBuffer.needsRead) {
      CONRADDEBUGASSERT(!itsScratchBuffer.needsFlush);
      itsIterator.readBuffer(itsScratchBuffer.vis, itsName);      
      itsScratchBuffer.needsRead=false;
  }
}

/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& TableBufferDataAccessor::rwVisibility()
{    
  fillBufferIfNeeded();
  itsScratchBuffer.needsFlush=true;
  return itsScratchBuffer.vis;      
}

/// set needsRead flag to true (i.e. used following an iterator step
/// to force updating the cache on the next data request
void TableBufferDataAccessor::notifyNewIteration() throw()
{
  itsScratchBuffer.needsRead=true;
}

/// sync the buffer with table if necessary
void TableBufferDataAccessor::sync()
{
  if (itsScratchBuffer.needsFlush) {
     // sync with the table
     itsIterator.writeBuffer(itsScratchBuffer.vis, itsName);
     itsScratchBuffer.needsFlush=false;
  }
}
