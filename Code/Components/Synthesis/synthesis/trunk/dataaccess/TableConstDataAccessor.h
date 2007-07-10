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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_CONST_DATA_ACCESSOR_H
#define TABLE_CONST_DATA_ACCESSOR_H

// own includes
#include <dataaccess/IConstDataAccessor.h>
#include <dataaccess/DataAccessorStub.h>
#include <dataaccess/DataAccessError.h>

namespace conrad {
	
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

  /// Visibilities (a cube is nRow x nChannel x nPol; each element is
  /// a complex visibility)
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  virtual const casa::Cube<casa::Complex>& visibility() const;

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
     
  /// @brief set itsXxxChanged flags corresponding to items updated on
  /// each iteration to true
  /// @details Such caches like visibility, uvw, noise and flags are updated
  /// on each new iteration. These are invalidated by call to this methid.
  /// Caches of frequency/velocity axis are updated less regularly (may be
  /// only once if the is just one spectral window in the measurement set).
  /// These are invalidated by a call to notifyNewSpectralWindow(), if
  /// the new window is not the same as the cached one
  virtual void invalidateIterationCaches() const throw();
  
  /// @brief set itsXxxChanged flags corresponding to spectral axis
  /// information to true
  /// @details See invalidateIterationCaches for more details
  void invalidateSpectralCaches() const throw();

  /// @brief Obtain a const reference to associated iterator.
  /// @details This method is mainly intended to be used in the derived
  /// non-const implementation, which works with a different type of the
  /// iterator.
  /// @return a const reference to the associated iterator
  const TableConstDataIterator& iterator() const throw(DataAccessLogicError);
private:  
  /// a reference to iterator managing this accessor
  const TableConstDataIterator& itsIterator;
  /// change flag for visibility
  mutable bool itsVisibilityChanged;
  /// internal buffer for visibility
  mutable casa::Cube<casa::Complex> itsVisibility;
  /// change flag for uvw
  mutable bool itsUVWChanged;
  /// internal buffer for uvw
  mutable casa::Vector<casa::RigidVector<casa::Double, 3> > itsUVW;
  /// change flag for frequency
  mutable bool itsFrequencyChanged;
  /// internal buffer for frequency
  mutable casa::Vector<casa::Double> itsFrequency;
  /// change flag for time
  mutable bool itsTimeChanged;
  /// internal buffer for time
  mutable casa::Double itsTime;
  /// change flag for the first antenna ids
  mutable bool itsAntenna1Changed;
  /// internal buffer for the first antenna ids
  mutable casa::Vector<casa::uInt> itsAntenna1;
  /// change flag for the second antenna ids
  mutable bool itsAntenna2Changed;
  /// internal buffer for the second antenna ids
  mutable casa::Vector<casa::uInt> itsAntenna2;
  /// change flag for the first feed ids
  mutable bool itsFeed1Changed;
  /// internal buffer for the first feed ids
  mutable casa::Vector<casa::uInt> itsFeed1;
  /// change flag for the second feed ids
  mutable bool itsFeed2Changed;
  /// internal buffer for the first feed ids
  mutable casa::Vector<casa::uInt> itsFeed2;
  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_CONST_DATA_ACCESSOR_H
