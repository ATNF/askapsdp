/// @file
/// @brief an implementation of IConstDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/TableDataAccessor.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataIterator.h>

using namespace conrad;
using namespace synthesis;

/// construct an object linked with the given const accessor
/// @param iter a reference to the associated accessor
TableDataAccessor::TableDataAccessor(const TableConstDataAccessor &acc) :
                 MetaDataAccessor(acc) {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableDataAccessor::visibility() const
{
  // active buffer should be returned
  fillBufferIfNeeded();
  return itsScratchBuffer.vis;  
}

/// read the information into the buffer if necessary
void TableDataAccessor::fillBufferIfNeeded() const
{
  if (itsScratchBuffer.needsRead) {
      CONRADDEBUGASSERT(!itsScratchBuffer.needsFlush);

      // a call to iterator method will be here
      //
      itsScratchBuffer.needsRead=false;
  }
}

/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& TableDataAccessor::rwVisibility()
{    
  // active buffer should be returned      
  fillBufferIfNeeded();
  itsScratchBuffer.needsFlush=true;
  return itsScratchBuffer.vis;      
 
  /*
  // original visibility is requested
  itsVisNeedsFlush=true;
  throw DataAccessLogicError("rwVisibility() for original visibilities is "
                                 "not yet implemented");  
  return const_cast<casa::Cube<casa::Complex>&>(getROAccessor().visibility());
  */
}


/// set needsFlush fkag to false (i.e. used after the visibility scratch 
/// buffer is synchronized with the disk)
void TableDataAccessor::notifySyncCompleted() throw()
{
  itsScratchBuffer.needsFlush=false;  
}

/// @return True if the visibilities need to be written back
bool TableDataAccessor::needSync() const throw()
{
  return itsScratchBuffer.needsFlush;
}


/// @brief invalidate items updated on each iteration
/// @details this method is overloaded to keep track of
/// the iterator position and supply an appropriate piece
/// of the active buffer. See TableConstDataAccessor for
/// details on this methid.
void TableDataAccessor::invalidateIterationCaches() const throw()
{
  //TableConstDataAccessor::invalidateIterationCaches();
  // may not need this method eventually. void for now.
}
