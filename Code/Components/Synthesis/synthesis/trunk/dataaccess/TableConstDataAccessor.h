/// @file
/// @brief an implementation of IConstDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor. It is intended to be used with both const and
/// non-const iterators. It is currently
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

/// own includes
#include <dataaccess/IConstDataAccessor.h>
#include <dataaccess/DataAccessorStub.h>

namespace conrad {
	
namespace synthesis {

/// to be able to link this class to appropriate iterator
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
     
  /// @brief set itsXxxChanged flags corresponding to items updated on
  /// each iteration to true
  /// @details Such caches like visibility, uvw, noise and flags are updated
  /// on each new iteration. These are invalidated by call to this methid.
  /// Caches of frequency/velocity axis are updated less regularly (may be
  /// only once if the is just one spectral window in the measurement set).
  /// These are invalidated by a call to notifyNewSpectralWindow(), if
  /// the new window is not the same as the cached one
  void invalidateIterationCaches() const throw();
  
  /// @brief set itsXxxChanged flags corresponding to spectral axis
  /// information to true
  /// @details See invalidateIterationCaches for more details
  void invalidateSpectralCaches() const throw();
protected:
  /// it's OK to have a protected data member here, because its read
  /// only 
  const TableConstDataIterator& itsIterator;
private:
  mutable bool itsVisibilityChanged;
  mutable casa::Cube<casa::Complex> itsVisibility;
  mutable bool itsUVWChanged;
  mutable casa::Vector<casa::RigidVector<casa::Double, 3> > itsUVW;
  mutable bool itsFrequencyChanged; 
  mutable casa::Vector<casa::Double> itsFrequency;
  mutable bool itsTimeChanged;
  mutable casa::Double itsTime;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_CONST_DATA_ACCESSOR_H
