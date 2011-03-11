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


#include <dataaccess/DataAccessorAdapter.h>
#include <askap/AskapError.h>
#include <dataaccess/DataAccessError.h>
// we need it just for NullDeleter
#include <askap/AskapUtil.h>

using namespace askap;
using namespace askap::accessors;


/// @brief construct detached accessor
DataAccessorAdapter::DataAccessorAdapter()  {}
  
 
/// @brief construct an object linked with the given const accessor
/// @param[in] acc shared pointer to a const accessor
DataAccessorAdapter::DataAccessorAdapter(const boost::shared_ptr<IConstDataAccessor> &acc) :
         itsAccessor(acc) 
{ 
  ASKAPCHECK(acc, "Attempting to initialise DataAccessorAdapter with a void shared pointer");  
}

/// @brief construct an object linked with the given non-const accessor
/// @param[in] acc shared pointer to a non-const accessor
DataAccessorAdapter::DataAccessorAdapter(const boost::shared_ptr<IDataAccessor> &acc) :
         itsAccessor(acc) 
{
  ASKAPCHECK(acc, "Attempting to initialise DataAccessorAdapter with a void shared pointer");
}

/// @brief construct an object linked with the given const accessor
/// @param[in] acc reference to a const accessor
/// @note it is a responsibility of a user of this class to ensure that the
/// reference is valid until the adapter is detached from it
DataAccessorAdapter::DataAccessorAdapter(const IConstDataAccessor &acc)
{
  // we don't access non-const methods anywhere, so constness is just conceptual here
  IConstDataAccessor* ptr = const_cast<IConstDataAccessor*>(&acc);
  itsAccessor.reset(ptr, utility::NullDeleter());
}

/// @brief construct an object linked with the given non-const accessor
/// @param[in] acc reference to a non-const accessor
/// @note it is a responsibility of a user of this class to ensure that the
/// reference is valid until the adapter is detached from it
DataAccessorAdapter::DataAccessorAdapter(IDataAccessor &acc)
{
  // just cast to the type stored in the shared pointer, read-write access relies
  // on dynamic cast back to the non-const type
  IConstDataAccessor* ptr = &acc;
  itsAccessor.reset(ptr, utility::NullDeleter());
}

/// @brief copy constructor 
/// @details It throws exception and is declared to avoid non-intentional 
/// copying by reference
/// @param[in] other object to copy from
DataAccessorAdapter::DataAccessorAdapter(const DataAccessorAdapter &other) : 
          IDataAccessor(other), itsAccessor(other.itsAccessor) 
{
  ASKAPTHROW(DataAccessLogicError, "Copy constructor is not supposed to be called for DataAccessorAdapter");
}

/// The number of rows in this chunk
/// @return the number of rows in this chunk
casa::uInt DataAccessorAdapter::nRow() const throw()
{
  return getROAccessor().nRow();
}

// The following methods implement metadata access
	
/// The number of spectral channels (equal for all rows)
/// @return the number of spectral channels
casa::uInt DataAccessorAdapter::nChannel() const throw()
{
  return getROAccessor().nChannel();
}


/// @brief The number of polarization products (equal for all rows)
/// @return the number of polarization products (can be 1,2 or 4)
casa::uInt DataAccessorAdapter::nPol() const throw()
{
  return getROAccessor().nPol();
}

/// @brief First antenna IDs for all rows
/// @return a vector with IDs of the first antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorAdapter::antenna1() const
{
  return getROAccessor().antenna1();
}

/// @brief Second antenna IDs for all rows
/// @return a vector with IDs of the second antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorAdapter::antenna2() const
{
  return getROAccessor().antenna2();
}

/// @brief First feed IDs for all rows
/// @return a vector with IDs of the first feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorAdapter::feed1() const
{
  return getROAccessor().feed1();
}

/// @brief Second feed IDs for all rows
/// @return a vector with IDs of the second feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorAdapter::feed2() const
{
  return getROAccessor().feed2();
}

/// @brief Position angles of the first feed for all rows
/// @return a vector with position angles (in radians) of the
/// first feed corresponding to each visibility
const casa::Vector<casa::Float>& DataAccessorAdapter::feed1PA() const
{
  return getROAccessor().feed1PA();
}


/// @brief Position angles of the second feed for all rows
/// @return a vector with position angles (in radians) of the
/// second feed corresponding to each visibility
const casa::Vector<casa::Float>& DataAccessorAdapter::feed2PA() const
{
  return getROAccessor().feed2PA();
}

/// @brief Return pointing centre directions of the first antenna/feed
/// @return a vector with direction measures (coordinate system
/// is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& DataAccessorAdapter::pointingDir1() const
{
  return getROAccessor().pointingDir1();
}


/// @brief Pointing centre directions of the second antenna/feed
/// @return a vector with direction measures (coordinate system
/// is is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& DataAccessorAdapter::pointingDir2() const
{
  return getROAccessor().pointingDir2();
}

/// @brief pointing direction for the centre of the first antenna 
/// @details The same as pointingDir1, if the feed offsets are zero
/// @return a vector with direction measures (coordinate system
/// is is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& DataAccessorAdapter::dishPointing1() const
{
  return getROAccessor().dishPointing1();
}

/// @brief pointing direction for the centre of the first antenna 
/// @details The same as pointingDir2, if the feed offsets are zero
/// @return a vector with direction measures (coordinate system
/// is is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& DataAccessorAdapter::dishPointing2() const
{
  return getROAccessor().dishPointing2();
}

/// Cube of flags corresponding to the output of visibility() 
/// @return a reference to nRow x nChannel x nPol cube with flag 
///         information. If True, the corresponding element is flagged.
const casa::Cube<casa::Bool>& DataAccessorAdapter::flag() const
{
  return getROAccessor().flag();
}


/// @brief UVW
/// @return a reference to vector containing uvw-coordinates
/// packed into a 3-D rigid vector
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
        DataAccessorAdapter::uvw() const
{
  return getROAccessor().uvw();
}
	
/// @brief uvw after rotation
/// @details This method calls UVWMachine to rotate baseline coordinates 
/// for a new tangent point. Delays corresponding to this correction are
/// returned by a separate method.
/// @param[in] tangentPoint tangent point to rotate the coordinates to
/// @return uvw after rotation to the new coordinate system for each row
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	         DataAccessorAdapter::rotatedUVW(const casa::MDirection &tangentPoint) const
{
  return getROAccessor().rotatedUVW(tangentPoint);
}	         
	         
/// @brief delay associated with uvw rotation
/// @details This is a companion method to rotatedUVW. It returns delays corresponding
/// to the baseline coordinate rotation. An additional delay corresponding to the 
/// translation in the tangent plane can also be applied using the image 
/// centre parameter. Set it to tangent point to apply no extra translation.
/// @param[in] tangentPoint tangent point to rotate the coordinates to
/// @param[in] imageCentre image centre (additional translation is done if imageCentre!=tangentPoint)
/// @return delays corresponding to the uvw rotation for each row
const casa::Vector<casa::Double>& DataAccessorAdapter::uvwRotationDelay(
	      const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const
{
  return getROAccessor().uvwRotationDelay(tangentPoint, imageCentre);
}	

/// @brief Noise level required for a proper weighting
/// @return a reference to nRow x nChannel x nPol cube with
///         complex noise estimates. Elements correspond to the
///         visibilities in the data cube.
const casa::Cube<casa::Complex>& DataAccessorAdapter::noise() const
{
  return getROAccessor().noise();
}


/// @brief Timestamp for each row
/// @return a timestamp for this buffer (it is always the same
///         for all rows. The timestamp is returned as 
///         Double w.r.t. the origin specified by the 
///         DataSource object and in that reference frame
casa::Double DataAccessorAdapter::time() const
{
  return getROAccessor().time();
}

/// @brief Frequency for each channel
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
const casa::Vector<casa::Double>& DataAccessorAdapter::frequency() const
{
  return getROAccessor().frequency();
}


/// @brief Velocity for each channel
/// @return a reference to vector containing velocities for each
///         spectral channel (vector size is nChannel). Velocities
///         are given as Doubles, the frame/units are specified by
///         the DataSource object (via IDataConverter).
const casa::Vector<casa::Double>& DataAccessorAdapter::velocity() const
{
  return getROAccessor().velocity();
}

/// @brief polarisation type for each product
/// @return a reference to vector containing polarisation types for
/// each product in the visibility cube (nPol() elements).
/// @note All rows of the accessor have the same structure of the visibility
/// cube, i.e. polarisation types returned by this method are valid for all rows.
const casa::Vector<casa::Stokes::StokesTypes>& DataAccessorAdapter::stokes() const
{
  return getROAccessor().stokes();
}

/// @brief read-only visibilities 
/// @details (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& DataAccessorAdapter::visibility() const
{
  return getROAccessor().visibility();
}
	
/// @brief Read-write access to visibilities 
/// @details (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& DataAccessorAdapter::rwVisibility()
{
  return getRWAccessor().rwVisibility();
}

/// @brief associate this adapter
/// @details This method associates the adapter with the given const accessor
/// @param[in] acc shared pointer to a valid accessor
void DataAccessorAdapter::associate(const boost::shared_ptr<IConstDataAccessor> &acc)
{
  ASKAPDEBUGASSERT(acc);
  itsAccessor = acc;
  itsAccessorChangeMonitor.notifyOfChanges();
}

/// @brief associate this adapter
/// @details This method associates the adapter with the given const accessor
/// @param[in] acc reference to a valid accessor
void DataAccessorAdapter::associate(const IConstDataAccessor &acc)
{
  // we don't access non-const methods anywhere, so constness is just conceptual here
  IConstDataAccessor* ptr = const_cast<IConstDataAccessor*>(&acc);
  itsAccessor.reset(ptr, utility::NullDeleter());
  itsAccessorChangeMonitor.notifyOfChanges();
}

/// @brief associate this adapter
/// @details This method associates the adapter with the given non-const accessor
/// @param[in] acc shared pointer to a valid accessor
void DataAccessorAdapter::associate(const boost::shared_ptr<IDataAccessor> &acc)
{
  ASKAPDEBUGASSERT(acc);
  itsAccessor = acc;
  itsAccessorChangeMonitor.notifyOfChanges();
}

/// @brief associate this adapter
/// @details This method associates the adapter with the given non-const accessor
/// @param[in] acc reference to a valid accessor
void DataAccessorAdapter::associate(IDataAccessor &acc)
{
  // just cast to the type stored in the shared pointer, read-write access relies
  // on dynamic cast back to the non-const type
  IConstDataAccessor* ptr = &acc;
  itsAccessor.reset(ptr, utility::NullDeleter());
  itsAccessorChangeMonitor.notifyOfChanges();
}

/// @brief check whether the adapter is associated with some accessor
/// @return true if adapter is associated with an accessor, false otherwise
bool DataAccessorAdapter::isAssociated() const
{
  return itsAccessor;
}

/// @brief detach adapter from an accessor
void DataAccessorAdapter::detach()
{
  itsAccessor.reset();
  itsAccessorChangeMonitor.notifyOfChanges();
}

/// @brief obtain a reference to associated const accessor
/// @details This method checks the validity of the shared pointer and
/// returns a reference of the const accessor type without any cast.
/// @return a refernce to associated const accessor
const IConstDataAccessor & DataAccessorAdapter::getROAccessor() const
{
  ASKAPCHECK(itsAccessor,"DataAccessorAdapter needs to be associated with a valid accessor before it can be used");
  return *itsAccessor;
}

/// @brief obtain a reference to associated non-const accessor
/// @details This method checks the validity of the shared pointer, casts
/// the pointer to a non-const type and return a reference. An exception
/// is thrown if the associated accessor is of the const type.
/// @return a refernce to associated non-const accessor
IDataAccessor & DataAccessorAdapter::getRWAccessor() const
{
  ASKAPCHECK(itsAccessor,"DataAccessorAdapter needs to be associated with a valid accessor before it can be used");
  boost::shared_ptr<IDataAccessor> rwAccessor = boost::dynamic_pointer_cast<IDataAccessor>(itsAccessor);
  ASKAPCHECK(rwAccessor, "DataAccessorAdapter needs to be associated with a non-const accessor for write operation");
  return *rwAccessor;
}


