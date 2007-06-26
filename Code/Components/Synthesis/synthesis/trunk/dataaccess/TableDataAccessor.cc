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

using namespace conrad;
using namespace synthesis;

/// construct an object linked with the given iterator
/// @param iter a reference to associated iterator
TableDataAccessor::TableDataAccessor(const TableConstDataIterator &iter) :
               TableConstDataAccessor(iter)  {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableDataAccessor::visibility() const
{
  return TableConstDataAccessor::visibility();
}


/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& TableDataAccessor::rwVisibility()
{
  return const_cast<casa::Cube<casa::Complex>&>(TableConstDataAccessor::visibility());
}
