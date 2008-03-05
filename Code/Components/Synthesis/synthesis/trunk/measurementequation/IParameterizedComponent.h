/// @file
///
/// @brief An abstract interface for a component depending on a number of 
///        parameters
/// @details
///     This interface does not implement any method. It is a structural type
///     allowing to hold a number of derived objects, which potentially
///     depend on a different number of parameters, in a container.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef I_PARAMETERIZED_COMPONENT_H
#define I_PARAMETERIZED_COMPONENT_H

// own includes
#include <measurementequation/IComponent.h>

// std includes
#include <string>

namespace askap {

namespace synthesis {

/// @brief An abstract interface for a component depending on a number of 
///        parameters
/// @details
///     This interface does not implement any method. It is a structural type
///     allowing to hold a number of derived objects, which potentially
///     depend on a different number of parameters, in a container.
/// @ingroup measurementequation  
struct IParameterizedComponent : virtual public IComponent {
  /// @brief get number of parameters
  /// @return a number of parameters  this component depends upon. 
  virtual size_t nParameters() const throw() = 0;
  
  /// @brief get the name of the given parameter
  /// @details All parameters are handled in the synthesis code using their
  /// string name, which allows to fix or free any of them easily. This method
  /// allows to obtain this string name using a integer index
  /// @param[in] index an integer index of the parameter (should be less than
  /// nParameters).
  /// @return a const reference to the string name of the parameter 
  virtual const std::string& parameterName(size_t index) const = 0;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef I_PARAMETERIZED_COMPONENT_H
