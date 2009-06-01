/// @file IConstDataAccessor.h
/// @brief Interface class for read-only access to visibility data
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
#ifndef I_CONST_DATA_ACCESSOR_H
#define I_CONST_DATA_ACCESSOR_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Quanta/MVDirection.h>
#include <measures/Measures/MDirection.h>
#include <scimath/Mathematics/RigidVector.h>
#include <measures/Measures/Stokes.h>


namespace askap {

namespace synthesis {

/// @brief Interface class for read-only access to visibility data
/// @details IConstDataAccessor is an interface class for read-only 
/// access to buffered visibility data. Working instances include 
/// a chunk of streamed data or a portion of the disk-based table. 
/// A reference to this type is returned by a derivative from
/// IConstDataIterator
/// @ingroup dataaccess_i
class IConstDataAccessor
{
public:
	
	/// Destruct
	virtual ~IConstDataAccessor();
		
	/// The number of rows in this chunk
	/// @return the number of rows in this chunk
	virtual casa::uInt nRow() const throw() = 0;

        // The following methods implement metadata access
		
	/// The number of spectral channels (equal for all rows)
	/// @return the number of spectral channels
	virtual casa::uInt nChannel() const throw() = 0;

	/// The number of polarization products (equal for all rows)
	/// @return the number of polarization products (can be 1,2 or 4)
	virtual casa::uInt nPol() const throw() = 0;

	/// First antenna IDs for all rows
	/// @return a vector with IDs of the first antenna corresponding
	/// to each visibility (one for each row)
	virtual const casa::Vector<casa::uInt>& antenna1() const = 0;

	/// Second antenna IDs for all rows
	/// @return a vector with IDs of the second antenna corresponding
	/// to each visibility (one for each row)
	virtual const casa::Vector<casa::uInt>& antenna2() const = 0;
	
        /// First feed IDs for all rows
	/// @return a vector with IDs of the first feed corresponding
	/// to each visibility (one for each row)
	virtual const casa::Vector<casa::uInt>& feed1() const = 0;

        /// Second feed IDs for all rows
	/// @return a vector with IDs of the second feed corresponding
	/// to each visibility (one for each row)
	virtual const casa::Vector<casa::uInt>& feed2() const = 0;

	/// Position angles of the first feed for all rows
	/// @return a vector with position angles (in radians) of the
	/// first feed corresponding to each visibility
	virtual const casa::Vector<casa::Float>& feed1PA() const = 0;

	/// Position angles of the second feed for all rows
	/// @return a vector with position angles (in radians) of the
	/// second feed corresponding to each visibility
	virtual const casa::Vector<casa::Float>& feed2PA() const = 0;

	/// Return pointing centre directions of the first antenna/feed
	/// @return a vector with direction measures (coordinate system
	/// is set via IDataConverter), one direction for each
	/// visibility/row
	virtual const casa::Vector<casa::MVDirection>& pointingDir1() const = 0;

        /// Pointing centre directions of the second antenna/feed
	/// @return a vector with direction measures (coordinate system
	/// is is set via IDataConverter), one direction for each
	/// visibility/row
	virtual const casa::Vector<casa::MVDirection>& pointingDir2() const = 0;

    /// pointing direction for the centre of the first antenna 
    /// @details The same as pointingDir1, if the feed offsets are zero
    /// @return a vector with direction measures (coordinate system
	/// is is set via IDataConverter), one direction for each
	/// visibility/row
	virtual const casa::Vector<casa::MVDirection>& dishPointing1() const = 0;

    /// pointing direction for the centre of the first antenna 
    /// @details The same as pointingDir2, if the feed offsets are zero
    /// @return a vector with direction measures (coordinate system
	/// is is set via IDataConverter), one direction for each
	/// visibility/row
	virtual const casa::Vector<casa::MVDirection>& dishPointing2() const = 0;

    /// Visibilities (a cube is nRow x nChannel x nPol; each element is
	/// a complex visibility)
	/// @return a reference to nRow x nChannel x nPol cube, containing
	/// all visibility data
	/// TODO:
	///     a non-const version to be able to subtract the model
	virtual const casa::Cube<casa::Complex>& visibility() const = 0;

	/// Cube of flags corresponding to the output of visibility() 
	/// @return a reference to nRow x nChannel x nPol cube with flag 
	///         information. If True, the corresponding element is flagged.
	virtual const casa::Cube<casa::Bool>& flag() const = 0;

	/// UVW
	/// @return a reference to vector containing uvw-coordinates
	/// packed into a 3-D rigid vector
	virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	        uvw() const = 0;
	
	/// @brief uvw after rotation
	/// @details This method calls UVWMachine to rotate baseline coordinates 
	/// for a new tangent point. Delays corresponding to this correction are
	/// returned by a separate method.
	/// @param[in] tangentPoint tangent point to rotate the coordinates to
	/// @return uvw after rotation to the new coordinate system for each row
	virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	         rotatedUVW(const casa::MDirection &tangentPoint) const = 0;
	         
	/// @brief delay associated with uvw rotation
	/// @details This is a companion method to rotatedUVW. It returns delays corresponding
	/// to the baseline coordinate rotation. An additional delay corresponding to the 
	/// translation in the tangent plane can also be applied using the image 
	/// centre parameter. Set it to tangent point to apply no extra translation.
	/// @param[in] tangentPoint tangent point to rotate the coordinates to
	/// @param[in] imageCentre image centre (additional translation is done if imageCentre!=tangentPoint)
	/// @return delays corresponding to the uvw rotation for each row
	virtual const casa::Vector<casa::Double>& uvwRotationDelay(
	         const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const = 0;
	

    /// Noise level required for a proper weighting
	/// @return a reference to nRow x nChannel x nPol cube with
	///         complex noise estimates. Elements correspond to the
	///         visibilities in the data cube.
	virtual const casa::Cube<casa::Complex>& noise() const = 0;

	/// Timestamp for each row
	/// @return a timestamp for this buffer (it is always the same
	///         for all rows. The timestamp is returned as 
	///         Double w.r.t. the origin specified by the 
	///         DataSource object and in that reference frame
	virtual casa::Double time() const = 0;

	/// Frequency for each channel
	/// @return a reference to vector containing frequencies for each
	///         spectral channel (vector size is nChannel). Frequencies
	///         are given as Doubles, the frame/units are specified by
	///         the DataSource object
	virtual const casa::Vector<casa::Double>& frequency() const = 0;

	/// Velocity for each channel
	/// @return a reference to vector containing velocities for each
	///         spectral channel (vector size is nChannel). Velocities
	///         are given as Doubles, the frame/units are specified by
	///         the DataSource object (via IDataConverter).
	virtual const casa::Vector<casa::Double>& velocity() const = 0;
	
	/// @brief polarisation type for each product
	/// @return a reference to vector containing polarisation types for
	/// each product in the visibility cube (nPol() elements).
	/// @note All rows of the accessor have the same structure of the visibility
	/// cube, i.e. polarisation types returned by this method are valid for all rows.
	virtual const casa::Vector<casa::Stokes::StokesTypes>& stokes() const = 0;
};

} // end of namespace synthesis

} // end of namespace askap

#endif // #ifndef I_CONST_DATA_ACCESSOR_H
