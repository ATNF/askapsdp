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

#ifndef NORMAL_EQUATIONS_TYPE_ERROR_H
#define NORMAL_EQUATIONS_TYPE_ERROR_H

#include <askap/AskapError.h>
#include <string>

namespace askap {

namespace synthesis {

/// @brief An error class passed in the exception during an attempt to use 
/// a wrong type of the normal equations class.
/// @details Previously, an instance of AskapError was thrown if 
/// ImagingMultiChunkEquation or GenericMultiChunkEquation encountered a
/// wrong type of normal equations. However, in composite equations
/// this particular error must be ignored, while other occurences also 
/// producing AskapError must not. This is a special exception class for
/// type error related to NormalEquations which allows a separate handling
/// of this particular type of exception
struct NormalEquationsTypeError: public askap::AskapError
{
  /// Constructor taking a message
  /// @param[in] message Message string
  explicit NormalEquationsTypeError(const std::string& message);
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef NORMAL_EQUATIONS_TYPE_ERROR_H
