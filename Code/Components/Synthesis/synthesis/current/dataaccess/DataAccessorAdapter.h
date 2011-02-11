/// @file
/// @brief an adapter to both IConstDataAccessor and IDataAccessor
///
/// @details This class is very similar to MetaDataAccessor. It essentially
/// implements the alternative approach mentioned in the documentation for
/// MetaDataAccessor, i.e. the original accessor is held by the shared pointer.
/// In principle, we could've used MetaDataAccessor instead of this class (or
/// convert all code using MetaDataAccessor to use this class). But in some
/// applications holding the original accessor by a reference leads to an
/// ugly design.
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
#ifndef DATA_ACCESSOR_ADAPTER_H
#define DATA_ACCESSOR_ADAPTER_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/IDataAccessor.h>


namespace askap {
	
namespace synthesis {


/// @brief an adapter to both IConstDataAccessor and IDataAccessor
/// @details This class is very similar to MetaDataAccessor. It essentially
/// implements the alternative approach mentioned in the documentation for
/// MetaDataAccessor, i.e. the original accessor is held by the shared pointer.
/// In principle, we could've used MetaDataAccessor instead of this class (or
/// convert all code using MetaDataAccessor to use this class). But in some
/// applications holding the original accessor by a reference leads to an
/// ugly design.
/// @ingroup dataaccess_hlp
class DataAccessorAdapter : virtual public IDataAccessor
{
public:
  // constructors
  
  /// @brief construct detached accessor
  DataAccessorAdapter();
 
  /// @brief construct an object linked with the given const accessor
  /// @param[in] acc shared pointer to a const accessor
  explicit DataAccessorAdapter(const boost::shared_ptr<IConstDataAccessor> &acc);

  /// @brief construct an object linked with the given non-const accessor
  /// @param[in] acc shared pointer to a const accessor
  explicit DataAccessorAdapter(const boost::shared_ptr<IDataAccessor> &acc);

  /// @brief copy constructor 
  /// @details It throws exception and is declared to avoid non-intentional 
  /// copying by reference
  /// @param[in] other object to copy from
  DataAccessorAdapter(const DataAccessorAdapter &other);
  
  // normal accessor methods

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
          
  /// @brief Noise level required for a proper weighting
  /// @return a reference to nRow x nChannel x nPol cube with
  ///         complex noise estimates. Elements correspond to the
  ///         visibilities in the data cube.
  virtual const casa::Cube<casa::Complex>& noise() const;

  /// @brief Timestamp for each row
  /// @return a timestamp for this buffer (it is always the same
  ///         for all rows. The timestamp is returned as 
  ///         Double w.r.t. the origin specified by the 
  ///         DataSource object and in that reference frame
  virtual casa::Double time() const;

  /// @brief Frequency for each channel
  /// @return a reference to vector containing frequencies for each
  ///         spectral channel (vector size is nChannel). Frequencies
  ///         are given as Doubles, the frame/units are specified by
  ///         the DataSource object
  virtual const casa::Vector<casa::Double>& frequency() const;

  /// @brief Velocity for each channel
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

  /// @brief read-only visibilities 
  /// @details (a cube is nRow x nChannel x nPol; 
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual const casa::Cube<casa::Complex>& visibility() const;
	
  /// @brief Read-write access to visibilities 
  /// @details (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual casa::Cube<casa::Complex>& rwVisibility();


  // methods to associate, detach and check status of this accessor
  
  /// @brief associate this adapter
  /// @details This method associates the adapter with the given const accessor
  /// @param[in] acc shared pointer to a valid accessor
  void associate(const boost::shared_ptr<IConstDataAccessor> &acc);

  /// @brief associate this adapter
  /// @details This method associates the adapter with the given non-const accessor
  /// @param[in] acc shared pointer to a valid accessor
  void associate(const boost::shared_ptr<IDataAccessor> &acc);
  
  /// @brief check whether the adapter is associated with some accessor
  /// @return true if adapter is associated with an accessor, false otherwise
  bool isAssociated() const;
  
  /// @brief detach adapter from an accessor
  void detach();
  
protected:
  /// @brief obtain a reference to associated const accessor
  /// @details This method checks the validity of the shared pointer and
  /// returns a reference of the const accessor type without any cast.
  /// @return a refernce to associated const accessor
  const IConstDataAccessor & getROAccessor() const;

  /// @brief obtain a reference to associated non-const accessor
  /// @details This method checks the validity of the shared pointer, casts
  /// the pointer to a non-const type and return a reference. An exception
  /// is thrown if the associated accessor is of the const type.
  /// @return a refernce to associated non-const accessor
  IDataAccessor & getRWAccessor() const;
      
private:
  /// @brief shared pointer to the associated accessor
  /// @note the accessor is kept as a const accessor and
  /// cast to non-const one if necessary
  boost::shared_ptr<IConstDataAccessor> itsAccessor;
};


} // namespace synthesis

} // namespace askap

#endif // DATA_ACCESSOR_ADAPTER_H

