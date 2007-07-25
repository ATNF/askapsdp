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
			   itsIterator(iter) {}

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
  return itsVisibility.value(itsIterator,
                        &TableConstDataIterator::fillVisibility);
}

/// UVW
/// @return a reference to vector containing uvw-coordinates
/// packed into a 3-D rigid vector
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
TableConstDataAccessor::uvw() const
{
  return itsUVW.value(itsIterator,&TableConstDataIterator::fillUVW);
}

/// Frequency for each channel
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
const casa::Vector<casa::Double>& TableConstDataAccessor::frequency() const
{
  return itsFrequency.value(itsIterator,&TableConstDataIterator::fillFrequency);
}

/// a helper adapter method to set the time via non-const reference
/// @param[in] time a reference to buffer to fill with the current time 
void TableConstDataAccessor::readTime(casa::Double &time) const
{
  time=itsIterator.getTime();
}
  

/// Timestamp for all rows
/// @return a timestamp for this buffer (it is always the same
///         for all rows. The timestamp is returned as 
///         Double w.r.t. the origin specified by the 
///         DataSource object and in that reference frame
casa::Double TableConstDataAccessor::time() const
{
  return itsTime.value(*this,&TableConstDataAccessor::readTime);
}

/// First antenna IDs for all rows
/// @return a vector with IDs of the first antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& TableConstDataAccessor::antenna1() const
{
  return itsAntenna1.value(itsIterator,&TableConstDataIterator::fillAntenna1);
}

/// Second antenna IDs for all rows
/// @return a vector with IDs of the second antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& TableConstDataAccessor::antenna2() const
{
  return itsAntenna2.value(itsIterator,&TableConstDataIterator::fillAntenna2);
}

/// First feed IDs for all rows
/// @return a vector with IDs of the first feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& TableConstDataAccessor::feed1() const
{
  return itsFeed1.value(itsIterator,&TableConstDataIterator::fillFeed1);
}

/// Second feed IDs for all rows
/// @return a vector with IDs of the second feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& TableConstDataAccessor::feed2() const
{
  return itsFeed1.value(itsIterator,&TableConstDataIterator::fillFeed2);
}

/// Return pointing centre directions of the first antenna/feed
/// @return a vector with direction measures (coordinate system
/// is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& TableConstDataAccessor::pointingDir1() 
                                        const
{
  return itsPointingDir1.value(itsIterator,
                               &TableConstDataIterator::fillPointingDir1);
}   

/// Return pointing centre directions of the second antenna/feed
/// @return a vector with direction measures (coordinate system
/// is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& TableConstDataAccessor::pointingDir2() 
                                        const
{
  return itsPointingDir2.value(itsIterator,
                               &TableConstDataIterator::fillPointingDir2);
}   
                                     

/// invalidate fields  updated on each iteration
void TableConstDataAccessor::invalidateIterationCaches() const throw()
{
  itsVisibility.invalidate();
  itsUVW.invalidate();
  itsTime.invalidate();
  itsAntenna1.invalidate();
  itsAntenna2.invalidate();
  itsFeed1.invalidate();
  itsFeed2.invalidate();
  itsPointingDir1.invalidate();
}

/// @brief invalidate all fields  corresponding to the spectral axis
/// @details See invalidateIterationCaches for more details
void TableConstDataAccessor::invalidateSpectralCaches() const throw()
{
  itsFrequency.invalidate();
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
