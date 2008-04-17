/// @file
/// @brief an implementation of IConstDataAccessor
///
/// @details TableConstDataAccessor is an implementation of the
/// DataAccessor working with TableConstDataIterator. It is currently
/// derived from DataAccessorStub as most of the
/// methods are stubbed. However, in the future
/// it should become a separate class derived
/// directly from its interface
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_CONST_DATA_ACCESSOR_H
#define TABLE_CONST_DATA_ACCESSOR_H

// own includes
#include <dataaccess/IConstDataAccessor.h>
#include <dataaccess/DataAccessorStub.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/CachedAccessorField.tcc>

namespace askap {
	
namespace synthesis {

/// to be able to link this class to appropriate iterator
/// @ingroup dataaccess_tab
class TableConstDataIterator;


/// @brief an implementation of IConstDataAccessor in the table-based case
///
/// @details TableConstDataAccessor is an implementation of the
/// DataAccessor working with TableConstDataIterator.
/// It is currently
/// derived from DataAccessorStub as most of the
/// methods are stubbed. However, in the future
/// it should become a separate class derived
/// directly from its interface
/// @ingroup dataaccess_tab
class TableConstDataAccessor : public DataAccessorStub
{
public:
  /// construct an object linked with the given iterator
  /// @param iter a reference to associated iterator
  explicit TableConstDataAccessor(const TableConstDataIterator &iter);

  /// The number of rows in this chunk
  /// @return the number of rows in this chunk
  virtual casa::uInt nRow() const throw();

  /// The number of spectral channels (equal for all rows)
  /// @return the number of spectral channels
  virtual casa::uInt nChannel() const throw();

  /// The number of polarization products (equal for all rows)
  /// @return the number of polarization products (can be 1,2 or 4)
  virtual casa::uInt nPol() const throw();
  
  /// Return pointing centre directions of the first antenna/feed
  /// @return a vector with direction measures (coordinate system
  /// is set via IDataConverter), one direction for each
  /// visibility/row
  virtual const casa::Vector<casa::MVDirection>& pointingDir1() const;

  /// Return pointing centre directions of the second antenna/feed
  /// @return a vector with direction measures (coordinate system
  /// is set via IDataConverter), one direction for each
  /// visibility/row
  virtual const casa::Vector<casa::MVDirection>& pointingDir2() const;

  /// pointing direction for the centre of the first antenna 
  /// @details The same as pointingDir1, if the feed offsets are zero
  /// @return a vector with direction measures (coordinate system
  /// is is set via IDataConverter), one direction for each
  /// visibility/row
  virtual const casa::Vector<casa::MVDirection>& dishPointing1() const;

  /// pointing direction for the centre of the first antenna 
  /// @details The same as pointingDir2, if the feed offsets are zero
  /// @return a vector with direction measures (coordinate system
  /// is is set via IDataConverter), one direction for each
  /// visibility/row
  virtual const casa::Vector<casa::MVDirection>& dishPointing2() const;


  /// Visibilities (a cube is nRow x nChannel x nPol; each element is
  /// a complex visibility)
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  virtual const casa::Cube<casa::Complex>& visibility() const;

  /// Cube of flags corresponding to the output of visibility() 
  /// @return a reference to nRow x nChannel x nPol cube with flag 
  ///         information. If True, the corresponding element is flagged.
  virtual const casa::Cube<casa::Bool>& flag() const;

  /// UVW
  /// @return a reference to vector containing uvw-coordinates
  /// packed into a 3-D rigid vector
  virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&uvw() const;

  /// Frequency for each channel
  /// @return a reference to vector containing frequencies for each
  ///         spectral channel (vector size is nChannel). Frequencies
  ///         are given as Doubles, the frame/units are specified by
  ///         the DataSource object
  virtual const casa::Vector<casa::Double>& frequency() const;

  /// Timestamp for each row
  /// @return a timestamp for this buffer (it is always the same
  ///         for all rows. The timestamp is returned as 
  ///         Double w.r.t. the origin specified by the 
  ///         DataSource object and in that reference frame
  virtual casa::Double time() const;
  
  /// First antenna IDs for all rows
  /// @return a vector with IDs of the first antenna corresponding
  /// to each visibility (one for each row)
  virtual const casa::Vector<casa::uInt>& antenna1() const;

  /// Second antenna IDs for all rows
  /// @return a vector with IDs of the second antenna corresponding
  /// to each visibility (one for each row)
  virtual const casa::Vector<casa::uInt>& antenna2() const;

  /// First feed IDs for all rows
  /// @return a vector with IDs of the first feed corresponding
  /// to each visibility (one for each row)
  virtual const casa::Vector<casa::uInt>& feed1() const;

  /// Second feed IDs for all rows
  /// @return a vector with IDs of the second feed corresponding
  /// to each visibility (one for each row)
  virtual const casa::Vector<casa::uInt>& feed2() const;

  /// @brief invalidate fields updated on each iteration
  /// @details Such caches like visibility, uvw, noise and flags are updated
  /// on each new iteration. These are invalidated by call to this methid.
  /// Caches of frequency/velocity axis are updated less regularly (may be
  /// only once if the is just one spectral window in the measurement set).
  /// These are invalidated by a call to notifyNewSpectralWindow(), if
  /// the new window is not the same as the cached one
  virtual void invalidateIterationCaches() const throw();
  
  /// @brief invalidate fields corresponding to the spectral axis
  /// @details See invalidateIterationCaches for more details
  void invalidateSpectralCaches() const throw();

  /// @brief Obtain a const reference to associated iterator.
  /// @details This method is mainly intended to be used in the derived
  /// non-const implementation, which works with a different type of the
  /// iterator.
  /// @return a const reference to the associated iterator
  const TableConstDataIterator& iterator() const throw(DataAccessLogicError);
private:  
  /// a helper adapter method to set the time via non-const reference
  /// @param[in] time a reference to buffer to fill with the current time 
  void readTime(casa::Double &time) const;
  
  /// a reference to iterator managing this accessor
  const TableConstDataIterator& itsIterator;
  
  /// internal buffer for visibility
  CachedAccessorField<casa::Cube<casa::Complex> > itsVisibility;
  
  /// internal buffer for flag
  CachedAccessorField<casa::Cube<casa::Bool> > itsFlag;
 
  /// internal buffer for uvw
  CachedAccessorField<casa::Vector<casa::RigidVector<casa::Double, 3> > > itsUVW;
 
  /// internal buffer for frequency
  CachedAccessorField<casa::Vector<casa::Double> > itsFrequency;

  /// internal buffer for time
  CachedAccessorField<casa::Double> itsTime;
  
  /// internal buffer for the first antenna ids
  CachedAccessorField<casa::Vector<casa::uInt> > itsAntenna1;

  /// internal buffer for the second antenna ids
  CachedAccessorField<casa::Vector<casa::uInt> > itsAntenna2;
  
  /// internal buffer for the first feed ids
  CachedAccessorField<casa::Vector<casa::uInt> > itsFeed1;
  
  /// internal buffer for the second feed ids
  CachedAccessorField<casa::Vector<casa::uInt> > itsFeed2;
  
  /// internal buffer for the pointing directions of the first antenna/feed
  CachedAccessorField<casa::Vector<casa::MVDirection> > itsPointingDir1;

  /// internal buffer for the pointing directions of the second antenna/feed
  CachedAccessorField<casa::Vector<casa::MVDirection> > itsPointingDir2;

  /// internal buffer for the pointing directions of the centre of the first antenna
  CachedAccessorField<casa::Vector<casa::MVDirection> > itsDishPointing1;

  /// internal buffer for the pointing directions of the centre of the second antenna
  CachedAccessorField<casa::Vector<casa::MVDirection> > itsDishPointing2;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef TABLE_CONST_DATA_ACCESSOR_H
