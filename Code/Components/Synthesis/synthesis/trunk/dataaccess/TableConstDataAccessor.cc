/// @file
/// @brief an implementation of IConstDataAccessor in the table-based case
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
using namespace conrad::synthesis;

/// construct an object linked with the given iterator
/// @param iter a reference to associated iterator
TableConstDataAccessor::TableConstDataAccessor(
                           const TableConstDataIterator &iter) :
			   itsIterator(iter), itsVisibilityChanged(true),
			   itsUVWChanged(true), itsFrequencyChanged(true),
			   itsTimeChanged(true), itsAntenna1Changed(true),
			   itsAntenna2Changed(true), itsFeed1Changed(true),
			   itsFeed2Changed(true) {}

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

/// Frequency for each channel
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
const casa::Vector<casa::Double>& TableConstDataAccessor::frequency() const
{
  if (itsFrequencyChanged) {
      itsIterator.fillFrequency(itsFrequency);
      itsFrequencyChanged=false;
  }
  return itsFrequency;
}

/// Timestamp for each row
/// @return a timestamp for this buffer (it is always the same
///         for all rows. The timestamp is returned as 
///         Double w.r.t. the origin specified by the 
///         DataSource object and in that reference frame
casa::Double TableConstDataAccessor::time() const
{
  if (itsTimeChanged) {
      itsTime=itsIterator.getTime();
      itsTimeChanged=false;
  }
  return itsTime;
}

/// First antenna IDs for all rows
/// @return a vector with IDs of the first antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& TableConstDataAccessor::antenna1() const
{
  if (itsAntenna1Changed) {
	 itsIterator.fillAntenna1(itsAntenna1);
	 itsAntenna1Changed=false;
  }
  return itsAntenna1;
}



/// set itsXxxChanged flags corresponding to items updated on each iteration to true
void TableConstDataAccessor::invalidateIterationCaches() const throw()
{
  itsVisibilityChanged=true;
  itsUVWChanged=true;
  itsTimeChanged=true;
  itsAntenna1Changed=true;
  itsAntenna2Changed=true;
  itsFeed1Changed=true;
  itsFeed2Changed=true;
}

/// @brief set itsXxxChanged flags corresponding to spectral axis
/// information to true
/// @details See invalidateIterationCaches for more details
void TableConstDataAccessor::invalidateSpectralCaches() const throw()
{
  itsFrequencyChanged=true;
}

/// @brief Obtain a const reference to associated iterator.
/// @details This method is mainly intended to be used in the derived
/// non-const implementation, which works with a different type of the
/// iterator.
/// @return a const reference to the associated iterator
const TableConstDataIterator& TableConstDataAccessor::iterator() const
                                            throw(DataAccessLogicError)
{
  return itsIterator;
}
