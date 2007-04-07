/// @file IDataAccessor.h
///
/// IDataAccessor: Interface class to access visibility data
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_DATA_ACCESSOR_H
#define I_DATA_ACCESSOR_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Quanta/MVDirection.h>
#include <scimath/Mathematics/RigidVector.h>


namespace conrad {

namespace synthesis {

/// IDataAccessor: an interface class to access buffered visibility data
/// working instances include a chunk of streamed data or a portion
/// of the disk-based table. A reference to this type is supposed to be
/// returned by a DataIterator object
class IDataAccessor
{
public:
	
	/// Destruct
	virtual ~IDataAccessor();
		
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
	/// is determined by the data accessor), one direction for each
	/// visibility/row
	virtual const casa::Vector<casa::MVDirection>& pointingDir1() const = 0;

        /// Pointing centre directions of the second antenna/feed
	/// @return a vector with direction measures (coordinate system
	/// is determined by the data accessor), one direction for each
	/// visibility/row
	virtual const casa::Vector<casa::MVDirection>& pointingDir2() const = 0;

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

        /// Noise level required for a proper weighting
	/// It is assumed at this stage that the same figure is valid for
	/// all spectral channels, although there could be a difference
	/// between different polarizations (hence a Matrix)
	/// @return a Matrix of complex noise estimates, rows correspond to
	///         the rows in this buffer, columns correspond to 
	///         polarizations (polarization conversion and selection
	///         are taken into account)
	virtual const casa::Matrix<casa::Complex>& noise() const = 0;

	/// Timestamp for each row
	/// @return a reference to vector containing timestamps for each
	///         row (as Double w.r.t. the origin specified by the 
	///         DataSource object along with the reference frame)  
	virtual const casa::Vector<casa::Double>& time() const = 0;

	/// Frequency for each channel
	/// @return a reference to vector containing frequencies for each
	///         spectral channel (vector size is nChannel). Frequencies
	///         are given as Doubles, the frame/units are specified by
	///         the DataSource object
	virtual const casa::Vector<casa::Double>& frequency() const = 0;
};

} // end of namespace synthesis

} // end of namespace conrad

#endif // #ifndef _I_DATA_ACCESSOR_H
