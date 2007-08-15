/// @file
///
/// @brief An abstract interface for a component depending on a number of 
///        parameters
/// @details
///     This interface does not implement any method. It is a structural type
///     allowing to hold a number of derived objects, which potentially
///     depend on a different number of parameters, in a container.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef I_PARAMETERIZED_COMPONENT_H
#define I_PARAMETERIZED_COMPONENT_H

#include <measurementequation/IComponent.h>


namespace conrad {

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
  /// @return a number of parameters  this component depends upon. This is
  /// a template parameter for this class
  virtual size_t nParameters() const throw() = 0;   
};

} // namespace synthesis

} // namespace conrad


#endif // #ifndef I_PARAMETERIZED_COMPONENT_H
