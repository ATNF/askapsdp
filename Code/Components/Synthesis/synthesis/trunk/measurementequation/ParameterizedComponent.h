/// @file
///
/// @brief An abstract component depending on a number of parameters
/// @details
///     This template does not implement calculate methods of the IComponent 
///     but encapsulates common functionality of all components depending 
///     on a number of free parameters. It holds the parameters in a
///     RigidVector.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef PARAMETERIZED_COMPONENT_H
#define PARAMETERIZED_COMPONENT_H

// own includes
#include <measurementequation/IParameterizedComponent.h>
#include <askap/AskapError.h>

// std includes
#include <string>

namespace askap {

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
      
  /// @brief get the name of the given parameter
  /// @details All parameters are handled in the synthesis code using their
  /// string name, which allows to fix or free any of them easily. This method
  /// allows to obtain this string name using a integer index
  /// @param[in] index an integer index of the parameter (should be less than
  /// nParameters).
  /// @return a const reference to the string name of the parameter 
  virtual const std::string& parameterName(size_t index) const
      { 
        ASKAPDEBUGASSERT(index<NComp); 
        return itsParameterNames(index);
      } 
      
  /// type of the vector with names
  typedef typename casa::RigidVector<std::string, NComp> ParameterNameVector;     
   
  /// @brief construct the object with a given parameters
  /// @details
  /// @param[in] param parameters of the component (meaning is defined in the
  /// derived classes)
  /// @param[in] names string names of the parameters (default is empty string)
  ParameterizedComponent(const casa::RigidVector<double, NComp> &param,
            const ParameterNameVector &names = ParameterNameVector(std::string())) :
            itsParameters(param), itsParameterNames(names) {}
  
  /// @brief construct the object with a given parameters and names
  /// @details
  /// @param[in] nameSuffix a suffix appended to all parameters name (useful to
  ///            identify the component
  /// @param[in] param parameters of the component (meaning is defined in the
  /// derived classes)
  /// @param[in] names names of the parameters. The type of this parameter is a
  /// template parameter of this (templated) method. It serves as an input iterator.
  /// The operator++ is called to advance to the next parameter, and the
  /// operator* is used to access the name. Name of each parameter should be 
  /// represented by a type convertible to std::string.
  template<typename Iter>
  ParameterizedComponent(const std::string &nameSuffix,
                const casa::RigidVector<double,NComp> &param,
                const Iter &names) :
         itsParameters(param), itsParameterNames(ParameterNameVector(nameSuffix))        
  { 
    Iter cName(names);
    for (size_t i=0; i<NComp; ++i,++cName) {
         itsParameterNames(i)=std::string(*cName) + itsParameterNames(i);
    } 
  }
   
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
         
  /// @brief access to names of the parameters from derived classes
  /// @details
  /// @return a reference to RigidVector of names
  inline const casa::RigidVector<std::string, NComp>& parameterNames() const throw()
         { return itsParameterNames;}
  
  /// @brief read-write access to names of the parameters from derived classes
  /// @details
  /// @return a non-const reference to RigidVector of names
  inline casa::RigidVector<std::string, NComp>& parameterNames() throw()
         { return itsParameterNames;}
                       
private:
  /// values of the parameters 
  casa::RigidVector<double, NComp> itsParameters;
  /// names of the parameters
  ParameterNameVector itsParameterNames;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef PARAMETERIZED_COMPONENT_H
