/// @file TableConstDataAccessor.h
///
/// TableConstDataAccessor:  an implementation of the DataAccessor to work
///                          with the TableConstDataIterator. It is currently
///                          derived from DataAccessorStub as most of the
///                          methods are stubbed. However, in the future
///                          it should become a separate class derived
///                          directly from its interface
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_CONST_DATA_ACCESSOR_H
#define TABLE_CONST_DATA_ACCESSOR_H

/// own includes
#include <dataaccess/IConstDataAccessor.h>
#include <dataaccess/DataAccessorStub.h>

namespace conrad {
	
namespace synthesis {

/// to be able to link this class to appropriate iterator
class TableConstDataIterator;

// derived to have the stubbed behavior for the remaining other methods
class TableConstDataAccessor : public DataAccessorStub
{
public:
  /// construct an object linked with the given iterator
  /// @param iter a reference to associated iterator
  TableConstDataAccessor(const TableConstDataIterator &iter);

  /// The number of rows in this chunk
  /// @return the number of rows in this chunk
  virtual casa::uInt nRow() const throw();

  /// The number of spectral channels (equal for all rows)
  /// @return the number of spectral channels
  virtual casa::uInt nChannel() const throw();

  /// The number of polarization products (equal for all rows)
  /// @return the number of polarization products (can be 1,2 or 4)
  virtual casa::uInt nPol() const throw();

  /// Visibilities (a cube is nRow x nChannel x nPol; each element is
  /// a complex visibility)
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  virtual const casa::Cube<casa::Complex>& visibility() const;

  /// UVW
  /// @return a reference to vector containing uvw-coordinates
  /// packed into a 3-D rigid vector
  virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&uvw() const;

  /// it's OK to have a protected data member here, because its read
  /// only 
  const TableConstDataIterator& itsIterator;
  /// set all itsXxxChanged flags to true
  void invalidateAllCaches() const throw();
private:
  mutable bool itsVisibilityChanged;
  mutable casa::Cube<casa::Complex> itsVisibility;
  mutable bool itsUVWChanged;
  mutable casa::Vector<casa::RigidVector<casa::Double, 3> > itsUVW;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_CONST_DATA_ACCESSOR_H
