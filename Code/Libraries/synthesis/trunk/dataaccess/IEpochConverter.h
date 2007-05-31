/// @file
///
/// IEpochConverter: Interface for epoch conversion. This is a relatively
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
#include <measures/Measures/MeasFrame.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace conrad {

namespace synthesis {

struct IEpochConverter : virtual public IConverterBase {
    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. Target units/frame are
    /// properties of the actual instance of the derived class
    virtual casa::Double operator()(const casa::MEpoch &in) const = 0;

    /// using statement to make this method public in all derived classes
    using IConverterBase::setMeasFrame;
};

} // namespace synthesis

} // namespace conrad

#endif // I_EPOCH_CONVERTER_H
