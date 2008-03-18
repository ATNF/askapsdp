/// @file
/// 
/// @brief A measurement equation, which generates a gaussian noise.
/// @details It is required for simulations to be able to add noise to
/// the simulated visibilities. To do it via measurement equations, one
/// has to create a composite measurement equation via SumOfTwoMEs class
/// with one of the input measurement equations set to an instance of
/// GaussianNoiseME defined here. If we need various similar classes the
/// approach probably needs to be changed to something similar to
/// CalibrationME template/effect classes.
/// @note The random number generator is a member of this class. In a 
/// parallel environment this would lead to a number of independent
/// generators used and to the same sequence generated in parallel 
/// branches of code. One needs a global solution (with an internode 
/// comminicaton on the cluster for a proper simulation of random numbers).
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GAUSSIAN_NOISE_ME_H
#define GAUSSIAN_NOISE_ME_H

// own includes
#include <measurementequation/IMeasurementEquation.h>

// casa includes
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/Complex.h>

namespace askap {

namespace synthesis {

/// @brief A measurement equation, which generates a gaussian noise.
/// @details It is required for simulations to be able to add noise to
/// the simulated visibilities. To do it via measurement equations, one
/// has to create a composite measurement equation via SumOfTwoMEs class
/// with one of the input measurement equations set to an instance of
/// GaussianNoiseME defined here. If we need various similar classes the
/// approach probably needs to be changed to something similar to
/// CalibrationME template/effect classes.
/// @note The random number generator is a member of this class. In a 
/// parallel environment this would lead to a number of independent
/// generators used and to the same sequence generated in parallel 
/// branches of code. One needs a global solution (with an internode 
/// comminicaton on the cluster for a proper simulation of random numbers).
/// @ingroup measurementequation
struct GaussianNoiseME : public IMeasurementEquation
{
  /// @brief constructor, initializes random distribution required.
  /// @param[in] variance required variance of the noise (same as rms
  /// here because the mean is always zero.
  /// @param[in] seed1 a first seed to initialize the random generator
  /// @param[in] seed2 a second seed to initialize the random generator 
  explicit GaussianNoiseME(double variance, casa::Int seed1 = 0, 
                                            casa::Int seed2 = 10);
  
  /// @brief Predict model visibilities for one accessor (chunk).
  /// @details This prediction is done for single chunk of data only. 
  /// It seems that all measurement equations should work with accessors 
  /// rather than iterators (i.e. the iteration over chunks should be 
  /// moved to the higher level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  virtual void predict(IDataAccessor &chunk) const;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This calculation is done for a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                          askap::scimath::INormalEquations& ne) const;
protected:

  /// @brief a helper method to obtain a random complex number.
  /// @details It runs the generator twice for real and imaginary part,
  /// composes a complex number and returns it.
  /// @return a random complex number
  casa::Complex getRandomComplexNumber() const;
  
private:
  /// @brief random number generator
  mutable casa::MLCG itsGen;
  /// @brief random number distrubiton
  mutable casa::Normal itsNoiseGen;
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef GAUSSIAN_NOISE_ME_H
