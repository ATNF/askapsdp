/// @file
///
/// @brief An unpolarized component (stokes Q,U and V always give 0)
/// @details
///     This is a derived template from ParameterizedComponent, which
///     represents an unpolarized component. It implements calculate
///     methods via new calculate methods, which don't have pol parameter
///     in their interface and always return stokes I. Having a separate
///     type allows to avoid unnecessary loops in polarization in
///     ComponentEquation, by testing the type with dynamic_cast. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef UNPOLARIZED_COMPONENT_H
#define UNPOLARIZED_COMPONENT_H

#include <measurementequation/ParameterizedComponent.h>
#include <measurementequation/IUnpolarizedComponent.h>

namespace askap {

namespace synthesis {

/// @brief An unpolarized component (stokes Q,U and V always give 0)
/// @details
///     This is a derived template from ParameterizedComponent, which
///     represents an unpolarized component. It implements calculate
///     methods via new calculate methods, which don't have pol parameter
///     in their interface and always return stokes I. Having a separate
///     type allows to avoid unnecessary loops in polarization in
///     ComponentEquation, by testing the type with dynamic_cast. 
/// @ingroup measurementequation  
template<size_t NComp>
struct UnpolarizedComponent : public ParameterizedComponent<NComp>,
                              virtual public IUnpolarizedComponent {
  
  /// @brief construct the object with a given parameters
  /// @details
  /// @param[in] param parameters of the component (meaning is defined in the
  /// derived classes)
  UnpolarizedComponent(const casa::RigidVector<double, NComp> &param) :
            ParameterizedComponent<NComp>(param) {}
  
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef UNPOLARIZED_COMPONENT_H
