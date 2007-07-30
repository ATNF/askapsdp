/// @file
/// @brief an implementation of IDataAccessor for original visibility
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor for original visibility working with TableDataIterator.
/// At this moment this class just throws an exception if a write is
/// attempted and mirrors all const functions.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/TableDataAccessor.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace conrad::synthesis;

/// construct an object linked with the given read-write iterator
/// @param iter a reference to the associated read-write iterator
TableDataAccessor::TableDataAccessor(const TableDataIterator &iter) :
                 MetaDataAccessor(iter.getAccessor()), 
                 itsNeedsFlushFlag(false), itsIterator(iter) {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableDataAccessor::visibility() const
{
  return getROAccessor().visibility();  
}

/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& TableDataAccessor::rwVisibility()
{    
  //throw DataAccessLogicError("rwVisibility() for original visibilities is "
  //                               "not yet implemented");  
  itsNeedsFlushFlag=true;
  // the solution with const_cast is not very elegant, however it seems to
  // be the only alternative to creating a copy of the buffer or making the whole
  // const interface untidy by putting a non-const method there. 
  // It is safe to use const_cast here because we know that the actual buffer
  // is declared mutable in CachedAccessorField.
  return const_cast<casa::Cube<casa::Complex>&>(getROAccessor().visibility());
}

/// this method flush back the data to disk if there are any changes
void TableDataAccessor::sync() const
{
  if (itsNeedsFlushFlag) {
      itsNeedsFlushFlag=false;
      itsIterator.writeOriginalVis();
  }
}
