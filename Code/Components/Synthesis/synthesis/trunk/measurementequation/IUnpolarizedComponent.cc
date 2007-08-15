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

// own includes
#include <measurementequation/IUnpolarizedComponent.h>
#include <measurementequation/IParameterizedComponent.h>

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
void conrad::synthesis::IUnpolarizedComponent::calculate(
                    const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    casa::Stokes::StokesTypes pol,
                    std::vector<double> &result) const
{
  if (stokesIndex(pol)) {
      // Q,U or V requested
      result.resize(2*freq.nelements(),0.);
  } else {
      // stokes I requested
      calculate(uvw,freq,result);
  }
}                    
  
/// @brief calculate visibilities and derivatives for this component
/// @details This variant of the method does simultaneous calculations of
/// the values and derivatives. The result is written to the provided buffer.
/// See the another version of this method for sizes/description of the buffer
/// structure.
/// @param[in] uvw  baseline spacings (in metres)
/// @param[in] freq vector of frequencies to do calculations for
/// @param[in] pol required polarization 
/// @param[out] result an output buffer used to store values
void conrad::synthesis::IUnpolarizedComponent::calculate(
                    const casa::RigidVector<casa::Double, 3> &uvw,
                    const casa::Vector<casa::Double> &freq,
                    casa::Stokes::StokesTypes pol,
                    std::vector<casa::AutoDiff<double> > &result) const
{
  if (stokesIndex(pol)) {
      // Q,U or V requested
      const conrad::synthesis::IParameterizedComponent *pc = 
             dynamic_cast<const conrad::synthesis::IParameterizedComponent*>(this);
      if (pc != NULL) {
          result.resize(2*freq.nelements(), casa::AutoDiff<double>(0.,
                        pc->nParameters()));
      }        
      result.resize(2*freq.nelements(), casa::AutoDiff<double>(0.,0));
  } else {
      // stokes I requested
      calculate(uvw,freq,result);
  }
}                                        
