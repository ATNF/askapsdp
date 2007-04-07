/// @file IBufferedAccessor.h
///
/// IBufferedAccessor: Interface class to access buffered visibility data
///        The only difference from IDataAccessor is that this type provide
///        scratch data "buffers" analogous to the model column in CASA.
///        
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_BUFFERED_ACCESSOR_H
#define I_BUFFERED_ACCESSOR_H

#include "IDataAccessor.h"

namespace conrad {

namespace synthesis {

/// IBufferedAccessor: an interface class to access buffered visibility data.
/// This is the same chunk of visibilities as one represented by IDataAccessor,
/// but with the optional scratch buffers. The behavior of these buffers 
/// depend on the DataSource. The table-based implementation is expected to
/// flush everything to disk. However, a streamed version of DataSource 
/// will probably remember the buffers over some limited period of time only.
class IBufferedAccessor : public IDataAccessor
{
public:
	
	/// The current number of scratch buffers. The bufferID takes
	/// values from 0 to nBuffers-1. A zero bufferID has a special 
	/// meaning and configures the accessor to return the original
	/// data 
	/// @return the number of scratch buffers available
	virtual casa::uInt nBuffers() const throw() = 0;

        /// Visibilities (a cube is nRow x nChannel x nPol; each element is
	/// a complex visibility)
	/// @return a reference to nRow x nChannel x nPol cube, containing
	/// all visibility data
	/// TODO:
	///     a non-const version to be able to subtract the model
	virtual const casa::Cube<casa::Complex>& visibility() const = 0;
};

} // end of namespace synthesis

} // end of namespace conrad

#endif // #ifndef I_BUFFERED_ACCESSOR_H
