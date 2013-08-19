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
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef ASKAP_ACCESSORS_META_DATA_ACCESSOR_H
#define ASKAP_ACCESSORS_META_DATA_ACCESSOR_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/IConstDataAccessor.h>


namespace askap {
	
namespace accessors {


/// @brief an adapter to most methods of IConstDataAccessor
///
/// @details IDataAccessor can represent buffers as well as the original data.
/// As a result, a number of objects derived from IDataAccessor must be
/// held by the iterator. This class implements an adapter which calls
/// methods of IConstDataAccessor intended for metadata access (there will
/// be just one instance of a class derived from IConstDataAccessor, but
/// many instances of a class derived from this one). Using this adapter allows to avoid
/// an unnecessary duplication of caches. Static data members are not a
/// suitable solution for this problem because there could be unrelated
/// instances of the iterator, which should have separate const accessors.
/// Possible derived classes include read-write accessor to buffers and
/// read-write accessor to original data.
/// @note an alternative approach is to split out all code managing metadata
/// into a separate class and to hold it in a shared pointer.
/// @ingroup dataaccess_hlp
class MetaDataAccessor : virtual public IConstDataAccessor
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

  /// Cube of flags corresponding to the output of visibility() 
  /// @return a reference to nRow x nChannel x nPol cube with flag 
  ///         information. If True, the corresponding element is flagged.
  virtual const casa::Cube<casa::Bool>& flag() const;

  /// UVW
  /// @return a reference to vector containing uvw-coordinates
  /// packed into a 3-D rigid vector
  virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
          uvw() const;
  
  /// @brief uvw after rotation
  /// @details This method calls UVWMachine to rotate baseline coordinates 
  /// for a new tangent point. Delays corresponding to this correction are
  /// returned by a separate method.
  /// @param[in] tangentPoint tangent point to rotate the coordinates to
  /// @return uvw after rotation to the new coordinate system for each row
  virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	         rotatedUVW(const casa::MDirection &tangentPoint) const;
	         
  /// @brief delay associated with uvw rotation
  /// @details This is a companion method to rotatedUVW. It returns delays corresponding
  /// to the baseline coordinate rotation. An additional delay corresponding to the 
  /// translation in the tangent plane can also be applied using the image 
  /// centre parameter. Set it to tangent point to apply no extra translation.
  /// @param[in] tangentPoint tangent point to rotate the coordinates to
  /// @param[in] imageCentre image centre (additional translation is done if imageCentre!=tangentPoint)
  /// @return delays corresponding to the uvw rotation for each row
  virtual const casa::Vector<casa::Double>& uvwRotationDelay(
	         const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const;
          

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
  
  /// @brief polarisation type for each product
  /// @return a reference to vector containing polarisation types for
  /// each product in the visibility cube (nPol() elements).
  /// @note All rows of the accessor have the same structure of the visibility
  /// cube, i.e. polarisation types returned by this method are valid for all rows.
  virtual const casa::Vector<casa::Stokes::StokesTypes>& stokes() const;
  
protected:
  /// @brief obtain a reference to associated const accessor
  /// @details (for use in derived methods)
  /// @return a refernce to associated const accessor
  const IConstDataAccessor & getROAccessor() const throw();
      
private:
  // a reference to the associated read-only accessor
  const IConstDataAccessor& itsROAccessor;
};


} // namespace accessors

} // namespace askap

#endif // #ifndef META_DATA_ACCESSOR_H
