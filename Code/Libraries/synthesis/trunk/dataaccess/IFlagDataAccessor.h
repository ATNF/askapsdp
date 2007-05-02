/// @file IFlagDataAccessor.h
///
/// IFlagDataAccessor: Interface class to access buffered visibility data
///        with the writing permission. This class is a further extension
///        of IDataAccessor to provide a read/write interface to the 
///        flag information. The user should dynamic cast to this 
///        interface from the reference or pointer returned by  
///        IDataIterator interface.
///        
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_FLAG_DATA_ACCESSOR_H
#define I_FLAG_DATA_ACCESSOR_H

#include "IDataAccessor.h"

namespace conrad {

namespace synthesis {

/// IFlagDataAccessor: a further extension of the IDataAccessor interface 
/// class with a read/write access to the flag information.
class IFlagDataAccessor : public IDataAccessor
{
public:
        /// Cube of flags corresponding to the output of visibility()
        /// @return a reference to nRow x nChannel x nPol cube with the flag
        ///         information. If True, the corresponding element is flagged.
        virtual const casa::Cube<casa::Bool>& flag() const = 0;

	/// Non-const access to the cube of flags.
        /// @return a reference to nRow x nChannel x nPol cube with the flag
        ///         information. If True, the corresponding element is flagged.
        virtual casa::Cube<casa::Bool>& rwFlag() = 0;
};

} // end of namespace synthesis

} // end of namespace conrad

#endif // #ifndef I_FLAG_DATA_ACCESSOR_H
