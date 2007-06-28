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
#ifndef TABLE_DATA_ACCESSOR_H
#define TABLE_DATA_ACCESSOR_H

// own includes
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/MetaDataAccessor.h>

namespace conrad {
	
namespace synthesis {

/// forward declaration of the type returned by reference
class TableDataIterator;

/// @brief an implementation of IDataAccessor for original visibility
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor for original visibility working with TableDataIterator.
/// At this moment this class just throws an exception if a write is
/// attempted and mirrors all const functions.
///
/// @note conceptually, the class should be derived from MetaDataAccessor and
/// IDataAccessor and MetaDataAccessor should be derived from IConstDataAccessor
/// rather then from IDataAccessor. However, the correct approach requires
/// a virtual inheritance from all interfaces and it won't work with g++-3.3
class TableDataAccessor : virtual public MetaDataAccessor
{
public:
  /// construct an object linked with the given const accessor
  /// @param acc a reference to the associated accessor
  explicit TableDataAccessor(const TableConstDataAccessor &acc);

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
