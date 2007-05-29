/// @file
///
/// IEpochConverter: Interface for epoch convertion. This is a relatively
/// low-level interface, which is used within the implementation of
/// the data accessor. The end user interacts with the IDataConverter
/// class. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_EPOCH_CONVERTER_H
#define I_EPOCH_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>

// own includes
#include "IConverterBase.h"

namespace conrad {

namespace synthesis {

struct IEpochConverter : public IConverterBase {
    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. Target units/frame are
    /// properties of the actual instance of the derived class
    virtual casa::Double operator()(const casa::MEpoch &in) const = 0;

    /// convert a given epoch into target units/frame
    /// @param in an epoch to convert. Target units/frame as well as
    /// the default units/frame are properties of the actual instance of
    /// the derived class.
    ///
    /// This method is supposed to be used with a table column, where
    /// the units and frame are defined once for the whole column
    virtual casa::Double operator()(const casa::Double &in) const = 0;
};

} // namespace synthesis

} // namespace conrad

#endif // I_EPOCH_CONVERTER_H
