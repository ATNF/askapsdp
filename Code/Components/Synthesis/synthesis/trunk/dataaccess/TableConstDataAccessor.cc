/// @file TableConstDataAccessor.cc
/// @brief an implementation of IConstDataAccessor working with TableConstDataIterator
///
/// @details TableConstDataAccessor is an implementation of the
/// DataAccessor working with the TableConstDataIterator. It is currently
/// derived from DataAccessorStub as most of the
/// methods are stubbed. However, in the future
/// it should become a separate class derived
/// directly from its interface
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/TableConstDataIterator.h>

using namespace conrad;
using namespace synthesis;

/// construct an object linked with the given iterator
/// @param iter a reference to associated iterator
TableConstDataAccessor::TableConstDataAccessor(const
                                            TableConstDataIterator &iter) :
			   itsIterator(iter), itsVisibilityChanged(true),
			   itsUVWChanged(true) {}

/// The number of rows in this chunk
/// @return the number of rows in this chunk
casa::uInt TableConstDataAccessor::nRow() const throw()
{
  return itsIterator.nRow();
}

/// The number of spectral channels (equal for all rows)
/// @return the number of spectral channels
casa::uInt TableConstDataAccessor::nChannel() const throw()
{
  return itsIterator.nChannel();
}

/// The number of polarization products (equal for all rows)
/// @return the number of polarization products (can be 1,2 or 4)
casa::uInt TableConstDataAccessor::nPol() const throw()
{
  return itsIterator.nPol();
}

/// Visibilities (a cube is nRow x nChannel x nPol; each element is
/// a complex visibility)
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
const casa::Cube<casa::Complex>& TableConstDataAccessor::visibility() const
{
  if (itsVisibilityChanged) {
      itsIterator.fillVisibility(itsVisibility);
      itsVisibilityChanged=false;
  }
  return itsVisibility;
}

/// UVW
/// @return a reference to vector containing uvw-coordinates
/// packed into a 3-D rigid vector
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
TableConstDataAccessor::uvw() const
{
  if (itsUVWChanged) {
      itsIterator.fillUVW(itsUVW);
      itsUVWChanged=false;
  }
  return itsUVW;
}

/// set all xxxChanged flags to true
void TableConstDataAccessor::invalidateAllCaches() const throw()
{
  itsVisibilityChanged=true;
  itsUVWChanged=true;
}
