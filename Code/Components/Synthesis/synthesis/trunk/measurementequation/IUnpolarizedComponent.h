/// @file
///
/// @brief An unpolarized component (stokes Q,U and V always give 0)
/// @details
///     This is a structural type describing an unpolarized component.
///     It implements calculate methods of the base interface via new 
///     calculate methods, which don't require pol parameter
///     and always return stokes I. Having a separate
///     type allows to avoid unnecessary loops in polarization in
///     ComponentEquation, by testing the type with dynamic_cast. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef I_UNPOLARIZED_COMPONENT_H
#define I_UNPOLARIZED_COMPONENT_H

#include <measurementequation/IComponent.h>
namespace conrad {

namespace synthesis {

/// @brief An unpolarized component (stokes Q,U and V always give 0)
/// @details
///     This is a structural type describing an unpolarized component.
///     It implements calculate methods of the base interface via new 
///     calculate methods, which don't require pol parameter
///     and always return stokes I. Having a separate
///     type allows to avoid unnecessary loops in polarization in
///     ComponentEquation, by testing the type with dynamic_cast. 
/// @ingroup measurementequation  
struct IUnpolarizedComponent : virtual public IComponent {
  
  
  /// @brief calculate stokes I visibilities for this component
  /// @details This variant of the method calculates just the visibilities
  /// (without derivatives) for a number of frequencies. This method has
  /// to be defined in the derived classes and is used to in the implementation
  /// of the IComponent interface if stokes I is requested. Otherwise result
  /// is filled with 0.
  /// @param[in] uvw  baseline spacings (in metres)
  /// @param[in] freq vector of frequencies to do calculations for
  /// @param[out] result an output buffer used to store values
  virtual void calculate(const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    std::vector<double> &result) const = 0;
  
  /// @brief calculate stokes I visibilities and derivatives for this component
  /// @details This variant of the method does simultaneous calculations of
  /// the values and derivatives. This method has
  /// to be defined in the derived classes and is used to in the implementation
  /// of the IComponent interface if stokes I is requested. Otherwise result
  /// is filled with 0.
  /// @param[in] uvw  baseline spacings (in metres)
  /// @param[in] freq vector of frequencies to do calculations for
  /// @param[out] result an output buffer used to store values
  virtual void calculate(const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    std::vector<casa::AutoDiff<double> > &result) const = 0;                    


  /// @brief calculate visibilities for this component
  /// @details This variant of the method calculates just the visibilities
  /// (without derivatives) for a number of frequencies. The result is stored 
  /// to the provided buffer, which is resized to twice the given 
  /// number of spectral points. Complex values are stored as two consequtive 
  /// double values. The first one is a real part, the second is imaginary part.
  /// @param[in] uvw  baseline spacings (in metres)
  /// @param[in] freq vector of frequencies to do calculations for
  /// @param[in] pol required polarization 
  /// @param[out] result an output buffer used to store values
  virtual void calculate(const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    casa::Stokes::StokesTypes pol,
                    std::vector<double> &result) const;
  
  /// @brief calculate visibilities and derivatives for this component
  /// @details This variant of the method does simultaneous calculations of
  /// the values and derivatives. The result is written to the provided buffer.
  /// See the another version of this method for sizes/description of the buffer
  /// structure.
  /// @param[in] uvw  baseline spacings (in metres)
  /// @param[in] freq vector of frequencies to do calculations for
  /// @param[in] pol required polarization 
  /// @param[out] result an output buffer used to store values
  virtual void calculate(const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    casa::Stokes::StokesTypes pol,
                    std::vector<casa::AutoDiff<double> > &result) const;                    

};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_UNPOLARIZED_COMPONENT_H
