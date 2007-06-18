/// @file IConverterBase.h
/// @brief A base class for all converter classes.
/// @details
/// IConverterBase: A base class for all converter classes. It doesn't
/// have any useful functionality and is used as a structural unit.
/// The only method defined is a virtual destructor to make the compiler
/// happy and reduce the number of *.cc files for the derived interfaces
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_CONVERTER_BASE_H
#define I_CONVERTER_BASE_H

// CASA includes
#include <measures/Measures/MeasFrame.h>

namespace conrad {

namespace synthesis {

/// @brief A base class for all converter classes.
/// @details It doesn't
/// have any useful functionality and is used as a structural unit.
/// The only method defined is a virtual destructor to make the compiler
/// happy and reduce the number of *.cc files for the derived interfaces
struct IConverterBase {

    /// an empty virtual destructor to keep the compiler happy
    /// for all derived interfaces
    virtual ~IConverterBase();

protected: // the following method(s) are not for a general framework user,
           // but rather for implementation
    /// set a frame (i.e. time and/or position), where the
    /// conversion is performed
    /// @param[in] frame  MeasFrame object (can be constructed from
    ///               MPosition or MEpoch on-the-fly)
    virtual void setMeasFrame(const casa::MeasFrame &frame) = 0;
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_CONVERTER_BASE_H
