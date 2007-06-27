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

using namespace conrad;
using namespace synthesis;

/// construct an object linked with the given iterator
/// @param iter a reference to associated iterator
TableDataAccessor::TableDataAccessor(const TableConstDataIterator &iter) :
               TableConstDataAccessor(iter), itsVisNeedsFlush(false) {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableDataAccessor::visibility() const
{
  if (itsScratchBufferPtr) {
      // active buffer should be returned
      fillBufferIfNeeded();
      return itsScratchBufferPtr->vis;      
  }
  // use original visibilities
  return TableConstDataAccessor::visibility();
}

/// read the information into the buffer if necessary
void TableDataAccessor::fillBufferIfNeeded() const
{
  // can't proceed if buffer is not set. Otherwise, it's a logic error
  CONRADDEBUGASSERT(itsScratchBufferPtr);  
  if (itsScratchBufferPtr->needsRead) {
      CONRADDEBUGASSERT(!itsScratchBufferPtr->needsFlush);

      // a call to iterator method will be here
      //
      itsScratchBufferPtr->needsRead=false;
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
  if (itsScratchBufferPtr) {
      // active buffer should be returned      
      fillBufferIfNeeded();
      itsScratchBufferPtr->needsFlush=true;
      return itsScratchBufferPtr->vis;      
  }
  // original visibility is requested
  itsVisNeedsFlush=true;
  throw DataAccessLogicError("rwVisibility() for original visibilities is "
                                 "not yet implemented");  
  return const_cast<casa::Cube<casa::Complex>&>(TableConstDataAccessor::visibility());
}


/// @brief set the scratch buffer to work with.
/// @details The scratch buffer is a cache of the visibility cube
/// and associated modification flags (one for reading, one for writing)
/// @param[in] buf a shared pointer to scratch buffer
void TableDataAccessor::setBuffer(
           const boost::shared_ptr<ScratchBuffer> &buf) throw()
{  
  itsScratchBufferPtr=buf;  
}

/// revert to original visibilities
void TableDataAccessor::setOriginal() throw()
{ 
  itsScratchBufferPtr.reset();
}


/// set itsVisNeedsFlush to false (i.e. used after the visibility scratch 
/// buffer is synchronized with the disk)
void TableDataAccessor::notifySyncCompleted() throw()
{
  itsVisNeedsFlush=false;
}

/// @return True if the visibilities need to be written back
bool TableDataAccessor::visNeedsSync() const throw()
{
  return itsVisNeedsFlush;
}


/// @brief invalidate items updated on each iteration
/// @details this method is overloaded to keep track of
/// the iterator position and supply an appropriate piece
/// of the active buffer. See TableConstDataAccessor for
/// details on this methid.
void TableDataAccessor::invalidateIterationCaches() const throw()
{
  TableConstDataAccessor::invalidateIterationCaches();
  // may not need this method eventually. void for now.
}

/// @brief Obtain a const reference to associated iterator.
/// @details See documentation for the base TableConstDataAccessor class
/// for more information
/// @return a const reference to the associated iterator
const TableDataIterator& TableDataAccessor::iterator()
                               const throw(DataAccessLogicError)
{
  #ifdef CONRAD_DEBUG
  try {
    return dynamic_cast<const TableDataIterator&>(
                  TableConstDataAccessor::iterator());    		   
  }
  catch (const std::bad_cast &bc) {
     throw DataAccessLogicError(bc.what());
  }
  #else
  return static_cast<const TableDataIterator&>(
                  TableConstDataAccessor::iterator());
  
  #endif
}
