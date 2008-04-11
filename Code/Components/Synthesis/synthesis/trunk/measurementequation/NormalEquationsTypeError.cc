/// @file
/// 
/// @brief An error class passed in the exception during an attempt to use 
/// a wrong type of the normal equations class.
/// @details Previously, an instance of AskapError was thrown if 
/// ImagingMultiChunkEquation or GenericMultiChunkEquation encountered a
/// wrong type of normal equations. However, in composite equations
/// this particular error must be ignored, while other occurences also 
/// producing AskapError must not. Here is a special exception class for
/// type error related to NormalEquations which allows a separate handling
/// of this particular type of exception

#include <measurementequation/NormalEquationsTypeError.h>

namespace askap {

namespace synthesis {

/// Constructor taking a message
/// @param[in] message Message string
NormalEquationsTypeError::NormalEquationsTypeError(const std::string& message) :
       AskapError(message) {}

} // namespace synthesis

} // namespace askap
