/// @file IDirectionConverter.h
/// @brief An interface for direction conversion.
/// @details This is a
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
#include <casa/Quanta/MVDirection.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace conrad {

namespace synthesis {

/// @brief An interface for direction conversion.
/// @details This is a
/// relatively low-level interface, which is used within the implementation
/// of the data accessor. The end user interacts with the IDataConverter
/// class. 
/// @ingroup dataaccess
struct IDirectionConverter : virtual public IConverterBase {
    /// convert specified MDirection to the target frame
    /// @param in an epoch to convert. Target frame is a
    /// property of the actual instance of the derived class
    virtual casa::MVDirection operator()(const casa::MDirection &in) const = 0;

    /// using statement to have setMeasFrame public.
    using IConverterBase::setMeasFrame;
};

} // namespace synthesis

} // namespace conrad

#endif // I_DIRECTION_CONVERTER_H
