/// @file
///
/// @brief An abstract component depending on a number of parameters
/// @details
///     This template does not implement calculate methods of the IComponent 
///     but encapsulates common functionality of all components depending 
///     on a number of free parameters. It holds the parameters in a
///     RigidVector.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef PARAMETERIZED_COMPONENT_H
#define PARAMETERIZED_COMPONENT_H

#include <measurementequation/IParameterizedComponent.h>

namespace conrad {

namespace synthesis {

/// @brief An abstract component depending on a number of parameters
/// @details
///     This template does not implement calculate methods of the IComponent 
///     but encapsulates common functionality of all components depending 
///     on a number of free parameters. It holds the parameters in a
///     RigidVector. Number of components is a template argument
/// @ingroup measurementequation  
template<size_t NComp>
class ParameterizedComponent : virtual public IParameterizedComponent {
public:

  /// @brief get number of parameters
  /// @return a number of parameters  this component depends upon. This is
  /// a template parameter for this class
  virtual size_t nParameters() const throw()
      { return NComp;}
   
  /// @brief construct the object with a given parameters
  /// @details
  /// @param[in] parameters of the component (meaning is defined in the
  /// derived classes)
  ParameterizedComponent(const casa::RigidVector<double, NComp> &param) :
            itsParameters(param) {}
   
protected:
  /// @brief access to parameters from derived classes
  /// @details
  /// @return a reference to RigidVector of parameters
  inline const casa::RigidVector<double, NComp>& parameters() const throw()
         {return itsParameters;}
   
  /// @brief read-write access to the parameters for derived classes
  /// @details
  /// @return a non-const reference to RigidVector of parameters
  inline casa::RigidVector<double, NComp>& parameters() throw()
         {return itsParameters;}         
private:
  /// values of parameters 
  casa::RigidVector<double, NComp> itsParameters; 
};

} // namespace synthesis

} // namespace conrad


#endif // #ifndef PARAMETERIZED_COMPONENT_H
