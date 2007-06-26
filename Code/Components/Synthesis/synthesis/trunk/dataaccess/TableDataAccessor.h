/// @file
/// @brief an implementation of IConstDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_DATA_ACCESSOR_H
#define TABLE_DATA_ACCESSOR_H

#include <dataaccess/TableConstDataAccessor.h>

namespace conrad {
	
namespace synthesis {

/// @brief an implementation of IConstDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator.
class TableDataAccessor : public TableConstDataAccessor
{
  /// construct an object linked with the given iterator
  /// @param iter a reference to associated iterator
  explicit TableDataAccessor(const TableConstDataIterator &iter);

  /// Read-only visibilities (a cube is nRow x nChannel x nPol; 
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual const casa::Cube<casa::Complex>& visibility() const;
  
  
  /// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual casa::Cube<casa::Complex>& rwVisibility();
  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_ACCESSOR_H
