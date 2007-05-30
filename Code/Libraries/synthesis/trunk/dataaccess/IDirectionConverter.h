/// @file
///
/// IDirectionConverter: Interface for direction conversion. This is a
/// relatively low-level interface, which is used within the implementation
/// of the data accessor. The end user interacts with the IDataConverter
/// class. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_DIRECTION_CONVERTER_H
#define I_DIRECTION_CONVERTER_H

// CASA includes
#include <measures/Measures/MDirection.h>

// own includes
#include "IConverterBase.h"
#include <casa/Quanta/MVDirection.h>

namespace conrad {

namespace synthesis {

struct IDirectionConverter : public IConverterBase {
    /// convert specified MDirection to the target frame
    /// @param in an epoch to convert. Target frame is a
    /// property of the actual instance of the derived class
    virtual casa::MVDirection operator()(const casa::MDirection &in) const = 0;
};

} // namespace synthesis

} // namespace conrad

#endif // I_DIRECTION_CONVERTER_H
