/// @file IDataAccessor.h
///
/// IDataAccessor: Interface class to access buffered visibility data
///        with the writing permission. It is ment to be used in conjunction 
///        with a read/write iterator (IDataIterator) for an access to 
///        associated buffers and optionally to update the visibilities if
///        the corresponding DataSource allows such operation.
///        
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_DATA_ACCESSOR_H
#define I_DATA_ACCESSOR_H

#include "IConstDataAccessor.h"

namespace conrad {

namespace synthesis {

/// IDataAccessor: an interface class to access buffered visibility data
/// with a read/write capability. It is meant to be used together with 
/// derived iterators, which support a read/write access (e.g. for buffers 
/// associated with visibility chunks).
class IDataAccessor : public IConstDataAccessor
{
public:
	
        /// Read-only visibilities (a cube is nRow x nChannel x nPol; 
	/// each element is a complex visibility)
	///
	/// @return a reference to nRow x nChannel x nPol cube, containing
	/// all visibility data
	///
	virtual const casa::Cube<casa::Complex>& visibility() const = 0;

	
        /// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
	/// each element is a complex visibility)
	///
	/// @return a reference to nRow x nChannel x nPol cube, containing
	/// all visibility data
	///
	virtual casa::Cube<casa::Complex>& rwVisibility() = 0;
};

} // end of namespace synthesis

} // end of namespace conrad

#endif // #ifndef I_DATA_ACCESSOR_H
