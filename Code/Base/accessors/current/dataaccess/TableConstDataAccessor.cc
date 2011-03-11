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

/// own includes
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/TableConstDataIterator.h>

using namespace askap;
using namespace askap::accessors;


/// construct an object linked with the given iterator
/// @param iter a reference to associated iterator
TableConstDataAccessor::TableConstDataAccessor(
                           const TableConstDataIterator &iter) :
			   itsIterator(iter), itsRotatedUVW(iter.uvwMachineCacheSize(),
			      iter.uvwMachineCacheTolerance()) {}

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

/// Cube of flags corresponding to the output of visibility() 
/// @return a reference to nRow x nChannel x nPol cube with flag 
///         information. If True, the corresponding element is flagged.
const casa::Cube<casa::Bool>& TableConstDataAccessor::flag() const
{
  return itsFlag.value(itsIterator, &TableConstDataIterator::fillFlag);
}

/// UVW
/// @return a reference to vector containing uvw-coordinates
/// packed into a 3-D rigid vector
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
TableConstDataAccessor::uvw() const
{
  return itsUVW.value(itsIterator,&TableConstDataIterator::fillUVW);
}

/// @brief uvw after rotation
/// @details This method calls UVWMachine to rotate baseline coordinates 
/// for a new tangent point. Delays corresponding to this correction are
/// returned by a separate method.
/// @param[in] tangentPoint tangent point to rotate the coordinates to
/// @return uvw after rotation to the new coordinate system for each row
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	           TableConstDataAccessor::rotatedUVW(const casa::MDirection &tangentPoint) const
{
  return itsRotatedUVW.uvw(*this, tangentPoint);
}	           
	         
/// @brief delay associated with uvw rotation
/// @details This is a companion method to rotatedUVW. It returns delays corresponding
/// to the baseline coordinate rotation. An additional delay corresponding to the 
/// translation in the tangent plane can also be applied using the image 
/// centre parameter. Set it to tangent point to apply no extra translation.
/// @param[in] tangentPoint tangent point to rotate the coordinates to
/// @param[in] imageCentre image centre (additional translation is done if imageCentre!=tangentPoint)
/// @return delays corresponding to the uvw rotation for each row
const casa::Vector<casa::Double>& TableConstDataAccessor::uvwRotationDelay(
	       const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const
{
  return itsRotatedUVW.delays(*this,tangentPoint,imageCentre);
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

/// Position angles of the first feed for all rows
/// @return a vector with position angles (in radians) of the
/// first feed corresponding to each visibility
const casa::Vector<casa::Float>& TableConstDataAccessor::feed1PA() const
{
  return itsFeed1PA.value(itsIterator,&TableConstDataIterator::fillFeed1PA);
}
     
/// Position angles of the second feed for all rows
/// @return a vector with position angles (in radians) of the
/// second feed corresponding to each visibility
const casa::Vector<casa::Float>& TableConstDataAccessor::feed2PA() const
{
  return itsFeed2PA.value(itsIterator,&TableConstDataIterator::fillFeed2PA);
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

/// pointing direction for the centre of the first antenna 
/// @details The same as pointingDir1, if the feed offsets are zero
/// @return a vector with direction measures (coordinate system
/// is is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& TableConstDataAccessor::dishPointing1() const
{
  return itsDishPointing1.value(itsIterator,
                               &TableConstDataIterator::fillDishPointing1);
}

/// pointing direction for the centre of the first antenna 
/// @details The same as pointingDir2, if the feed offsets are zero
/// @return a vector with direction measures (coordinate system
/// is is set via IDataConverter), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& TableConstDataAccessor::dishPointing2() const
{
  return itsDishPointing2.value(itsIterator,
                               &TableConstDataIterator::fillDishPointing2);
}

/// Noise level required for a proper weighting
/// @return a reference to nRow x nChannel x nPol cube with
///         complex noise estimates
const casa::Cube<casa::Complex>& TableConstDataAccessor::noise() const
{
  return itsNoise.value(itsIterator, &TableConstDataIterator::fillNoise);
}
  
/// Velocity for each channel
/// @return a reference to vector containing velocities for each
///         spectral channel (vector size is nChannel). Velocities
///         are given as Doubles, the frame/units are specified by
///         the DataSource object (via IDataConverter).
const casa::Vector<casa::Double>& TableConstDataAccessor::velocity() const
{
  throw AskapError("TableConstDataAccessor::velocity has not been implemented.");
}
 
/// @brief polarisation type for each product
/// @return a reference to vector containing polarisation types for
/// each product in the visibility cube (nPol() elements).
/// @note All rows of the accessor have the same structure of the visibility
/// cube, i.e. polarisation types returned by this method are valid for all rows.
const casa::Vector<casa::Stokes::StokesTypes>& TableConstDataAccessor::stokes() const
{
  return itsStokes.value(itsIterator, &TableConstDataIterator::fillStokes);
}                                    

/// invalidate fields  updated on each iteration
void TableConstDataAccessor::invalidateIterationCaches() const throw()
{
  itsVisibility.invalidate();
  itsFlag.invalidate();
  itsUVW.invalidate();
  itsRotatedUVW.invalidate();
  itsTime.invalidate();
  itsAntenna1.invalidate();
  itsAntenna2.invalidate();
  itsFeed1.invalidate();
  itsFeed2.invalidate();
  itsFeed1PA.invalidate();
  itsFeed2PA.invalidate();
  itsPointingDir1.invalidate();
  itsPointingDir2.invalidate();
  itsDishPointing1.invalidate();
  itsDishPointing2.invalidate();
  itsNoise.invalidate();
}

/// @brief invalidate all fields  corresponding to the spectral axis
/// @details See invalidateIterationCaches for more details
void TableConstDataAccessor::invalidateSpectralCaches() const throw()
{
  itsFrequency.invalidate();
  // polarisation info is attached to a spectral info (i.e. both are controlled by 
  // data descriptor ID, which is a sort of correlator setup ID)
  itsStokes.invalidate();
}

/// @brief invalidate cache of rotated uvw and delays
/// @details Cache of rotated uvw and delays is kept per accessor, need this
/// method to access private field
void TableConstDataAccessor::invalidateRotatedUVW() const throw()
{
  itsRotatedUVW.invalidate();
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
