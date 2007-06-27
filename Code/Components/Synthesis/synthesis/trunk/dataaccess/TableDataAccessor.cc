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
               TableConstDataAccessor(iter), itsBufferNeedsFlush(false),
	       itsBufferChanged(true) {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableDataAccessor::visibility() const
{
  if (itsBufferedVisibility) {
      // active buffer should be returned
      fillBufferIfNeeded();
      return *itsBufferedVisibility;      
  }
  // use original visibilities
  return TableConstDataAccessor::visibility();
}

/// read the information into the buffer if necessary
void TableDataAccessor::fillBufferIfNeeded() const
{
  if (itsBufferChanged) {
      // can't proceed if buffer is not set. Otherwise, it's a logic error
      CONRADDEBUGASSERT(itsBufferedVisibility);

      // a call to iterator method will be here
      //
      itsBufferChanged=false;
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
  itsBufferNeedsFlush=true;
  if (itsBufferedVisibility) {
      // active buffer should be returned
      fillBufferIfNeeded();
      return *itsBufferedVisibility;      
  }
  throw DataAccessLogicError("rwVisibility() for original visibilities is "
                                 "not yet implemented");  
  return const_cast<casa::Cube<casa::Complex>&>(TableConstDataAccessor::visibility());
}


/// @brief set the buffer to work with
/// @details The modification flag (describing whether one needs to
/// flush the buffer back to disk) is reset automatically.
/// @param[in] buf shared pointer to nRow x nChannel x nPol cube with
/// visibilities
void TableDataAccessor::setBuffer(
           const boost::shared_ptr<casa::Cube<casa::Complex> > &buf) throw()
{
  itsBufferedVisibility=buf;
  resetBufferFlushFlag();
}

/// revert to original visibilities
void TableDataAccessor::setOriginal() throw()
{
  itsBufferedVisibility.reset();
}


/// set itsBufferNeedsFlush to false (i.e. after this buffer is
/// synchronized with the disk
void TableDataAccessor::resetBufferFlushFlag() throw()
{
  itsBufferNeedsFlush=false;
}

/// @return True if the active buffer needs to be written back
bool TableDataAccessor::bufferNeedsFlush() const throw()
{
  return itsBufferNeedsFlush;
}
