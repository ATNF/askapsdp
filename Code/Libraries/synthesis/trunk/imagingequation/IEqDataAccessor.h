/// @file
///
/// IEqDataAccessor: Access to buffered visibility data
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef IEQDATAACCESSOR_H_
#define IEQDATAACCESSOR_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/MVDirection.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>

namespace conrad {

class IEqDataAccessor
{
public:
	/// Construct
	IEqDataAccessor();
	
	/// Destruct
	virtual ~IEqDataAccessor();
	
	/// Initialize model column
	virtual void initmodel();

	/// The number of rows in this chunk
	/// @return the number of rows in this chunk
	virtual casa::uInt nRow() const throw() = 0;

        // The following methods implement metadata access
		
	/// The number of spectral channels (equal for all rows)
	/// @return the number of spectral channels
	virtual casa::uInt nChannel() const throw() = 0;

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

        /// Visibilities (a matrix is nRow x nChannel; each element is
	/// a stokes vector)
	/// @return a reference to nRow x nChannel matrix, containing
	/// all visibility data
	/// TODO:
	///     a non-const version to be able to subtract the model
	virtual const casa::Matrix<casa::CStokesVector>& visibility() const = 0;

	/// Matrix of flags
	/// @return a reference to nRow x nChannel matrix with flag information
	/// if True, the corresponding element is flagged.
	virtual const casa::Matrix<casa::Bool>& flag() const = 0;

	/// UVW
	/// @return a reference to vector containing uvw-coordinates
	/// packed into a 3-D rigid vector
	virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	        uvw() const = 0;
};

} // end of namespace conrad

#endif /*IEQDATAACCESSOR_H_*/
