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
  // original visibility is requested
  throw DataAccessLogicError("rwVisibility() for original visibilities is "
                                 "not yet implemented");  
  return const_cast<casa::Cube<casa::Complex>&>(getROAccessor().visibility());
}
