/// @file
///
/// @brief Abstract component
/// @details
/// IComponent is a base class for components working with ComponentEquation
/// examples of components include, e.g. Gaussian or point sources.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 
#ifndef I_COMPONENT_H
#define I_COMPONENT_H

// std includes
#include <vector>

// casa includes
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>
#include <casa/Arrays/Vector.h>
#include <scimath/Mathematics/RigidVector.h>



namespace conrad {

namespace synthesis {

/// @brief Abstract component
/// @details
/// IComponent is a base class for components working with ComponentEquation
/// examples of components include, e.g. Gaussian or point sources.
/// @note Overloaded virtual function calculate most likely will call
/// templated method for casa::Double and casa::AutoDiff<casa::Double>.
/// We can't have templated method & polymorphism together. 
/// @ingroup measurementequation  
struct IComponent {
  /// virtual destructor to keep the compiler happy
  virtual ~IComponent(); 
  
  /// @brief calculate visibilities for this component
  /// @details This variant of the method calculates just the visibilities
  /// (without derivatives) for a number of frequencies. The result is stored 
  /// to the provided buffer, which is resized to twice the given 
  /// number of spectral points. Complex values are stored as two consequtive 
  /// double values. The first one is a real part, the second is imaginary part.
  /// @param[in] uvw  baseline spacings (in metres)
  /// @param[in] freq vector of frequencies to do calculations for
  /// @param[out] result an output buffer used to store values
  virtual void calculate(const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    std::vector<double> &result) const = 0;
  
  /// @brief calculate visibilities and derivatives for this component
  /// @details This variant of the method does simultaneous calculations of
  /// the values and derivatives. The result is written to the provided buffer.
  /// See the another version of this method for sizes/description of the buffer
  /// structure.
  /// @param[in] uvw  baseline spacings (in metres)
  /// @param[in] freq vector of frequencies to do calculations for
  /// @param[out] result an output buffer used to store values
  virtual void calculate(const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    std::vector<casa::AutoDiff<double> > &result) const = 0;                    
}; 

} // namespace synthesis

} // namespace conrad


#endif // #ifndef I_COMPONENT_H

