/// @file IEpochConverter.h
/// @brief An interface for epoch conversion
/// @details This is a relatively
/// low-level interface, which is used within the implementation of
/// the data accessor. The end user interacts with the IDataConverter
/// class. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_EPOCH_CONVERTER_H
#define I_EPOCH_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace askap {

namespace synthesis {

/// @brief An interface for epoch conversion
/// @details This is a relatively
/// low-level interface, which is used within the implementation of
/// the data accessor. The end user interacts with the IDataConverter
/// class.
/// @note inherited setMeasFrame method is public, rather than protected.
/// Apparently, doxygen doesn't know how the using statement works in C++.
/// @ingroup dataaccess_conv
struct IEpochConverter : virtual public IConverterBase {
    /// convert specified MEpoch to the target units/frame
    /// @param[in] in an epoch to convert. Target units/frame are
    /// properties of the actual instance of the derived class
    virtual casa::Double operator()(const casa::MEpoch &in) const = 0;

    /// Reverse conversion (casa::Double to full measure)
    /// @param[in] in an epoch given as Double in the target units/frame
    /// @return the same epoch as a fully qualified measure
    virtual casa::MEpoch toMeasure(casa::Double in) const = 0;

    /// Reverse conversion (casa::MVEpoch to full measure)
    /// @param[in] in an epoch given as MVEpoch in the target frame
    /// @return the same epoch as a fully qualified measure
    virtual casa::MEpoch toMeasure(const casa::MVEpoch &in) const throw() = 0;

    /// using statement to make this method public in all derived classes
    using IConverterBase::setMeasFrame;
};

} // namespace synthesis

} // namespace askap

#endif // I_EPOCH_CONVERTER_H
