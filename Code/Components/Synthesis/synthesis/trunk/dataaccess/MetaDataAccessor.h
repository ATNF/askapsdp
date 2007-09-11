/// @file
/// @brief an adapter to most methods of IConstDataAccessor
///
/// @details IDataAccessor can represent buffers as well as the original data.
/// As a result, a number of objects derived from IDataAccessor must be
/// held by the iterator. This class implements an adapter which calls
/// methods of IConstDataAccessor associated to metadata access (there will
/// be just one instance of a class derived from IConstDataAccessor, but
/// many instances of a class derived from this one). Using this adapter allows to avoid
/// an unnecessary duplication of caches. Static data members are not a
/// suitable solution for this problem because there could be unrelated
/// instances of the iterator, which should have separate const accessors.
/// Possible derived classes include read-write accessor to buffers and
/// read-write accessor to original data.
/// @note an alternative approach is to split out all code managing metadata
/// into a separate class and to hold it in a shared pointer.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef META_DATA_ACCESSOR_H
#define META_DATA_ACCESSOR_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/IDataAccessor.h>


namespace conrad {
	
namespace synthesis {


/// @brief an adapter to most methods of IConstDataAccessor
///
/// @details IDataAccessor can represent buffers as well as the original data.
/// As a result, a number of objects derived from IDataAccessor must be
/// held by the iterator. This class implements an adapter which calls
/// methods of IConstDataAccessor associated to metadata access (there will
/// be just one instance of a class derived from IConstDataAccessor, but
/// many instances of a class derived from this one). Using this adapter allows to avoid
/// an unnecessary duplication of caches. Static data members are not a
/// suitable solution for this problem because there could be unrelated
/// instances of the iterator, which should have separate const accessors.
/// Possible derived classes include read-write accessor to buffers and
/// read-write accessor to original data.
/// @note an alternative approach is to split out all code managing metadata
/// into a separate class and to hold it in a shared pointer.
/// Conceptually, this class has to be derived from IConstDataAccessor.
/// However, it would cause problems with g++-3.3 later. 
/// @ingroup dataaccess_hlp
class MetaDataAccessor : virtual public IDataAccessor
{
public:
  /// construct an object linked with the given const accessor
  /// @param acc a reference to the associated accessor
  explicit MetaDataAccessor(const IConstDataAccessor &acc);

  /// The number of rows in this chunk
  /// @return the number of rows in this chunk
  virtual casa::uInt nRow() const throw();
  	
  /// The number of spectral channels (equal for all rows)
  /// @return the number of spectral channels
  virtual casa::uInt nChannel() const throw();

  /// The number of polarization products (equal for all rows)
  /// @return the number of polarization products (can be 1,2 or 4)
  virtual casa::uInt nPol() const throw();

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

  /// Position angles of the first feed for all rows
  /// @return a vector with position angles (in radians) of the
  /// first feed corresponding to each visibility
  virtual const casa::Vector<casa::Float>& feed1PA() const;

  /// Position angles of the second feed for all rows
  /// @return a vector with position angles (in radians) of the
  /// second feed corresponding to each visibility
  virtual const casa::Vector<casa::Float>& feed2PA() const;

  /// Return pointing centre directions of the first antenna/feed
  /// @return a vector with direction measures (coordinate system
  /// is set via IDataConverter), one direction for each
  /// visibility/row
  virtual const casa::Vector<casa::MVDirection>& pointingDir1() const;

  /// Pointing centre directions of the second antenna/feed
  /// @return a vector with direction measures (coordinate system
  /// is is set via IDataConverter), one direction for each
  /// visibility/row
  virtual const casa::Vector<casa::MVDirection>& pointingDir2() const;

  /// Cube of flags corresponding to the output of visibility() 
  /// @return a reference to nRow x nChannel x nPol cube with flag 
  ///         information. If True, the corresponding element is flagged.
  virtual const casa::Cube<casa::Bool>& flag() const;

  /// UVW
  /// @return a reference to vector containing uvw-coordinates
  /// packed into a 3-D rigid vector
  virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
          uvw() const;

  /// Noise level required for a proper weighting
  /// @return a reference to nRow x nChannel x nPol cube with
  ///         complex noise estimates. Elements correspond to the
  ///         visibilities in the data cube.
  virtual const casa::Cube<casa::Complex>& noise() const;

  /// Timestamp for each row
  /// @return a timestamp for this buffer (it is always the same
  ///         for all rows. The timestamp is returned as 
  ///         Double w.r.t. the origin specified by the 
  ///         DataSource object and in that reference frame
  virtual casa::Double time() const;

  /// Frequency for each channel
  /// @return a reference to vector containing frequencies for each
  ///         spectral channel (vector size is nChannel). Frequencies
  ///         are given as Doubles, the frame/units are specified by
  ///         the DataSource object
  virtual const casa::Vector<casa::Double>& frequency() const;

  /// Velocity for each channel
  /// @return a reference to vector containing velocities for each
  ///         spectral channel (vector size is nChannel). Velocities
  ///         are given as Doubles, the frame/units are specified by
  ///         the DataSource object (via IDataConverter).
  virtual const casa::Vector<casa::Double>& velocity() const;
protected:
  /// @brief obtain a reference to associated const accessor
  /// @details (for use in derived methods)
  /// @return a refernce to associated const accessor
  const IConstDataAccessor & getROAccessor() const throw();
  
private:
  // a reference to the associated read-only accessor
  const IConstDataAccessor & itsROAccessor;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef META_DATA_ACCESSOR_H
