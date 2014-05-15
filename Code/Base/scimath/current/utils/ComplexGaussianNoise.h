/// @file
///
/// Wraps around casacore's random generators to provide complex noise generator with
/// a given variance.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ASKAP_SCIMATH_UTILS_COMPLEX_GAUSSIAN_NOISE_H
#define ASKAP_SCIMATH_UTILS_COMPLEX_GAUSSIAN_NOISE_H

// casa includes
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/Complex.h>

namespace askap {

namespace scimath {

/// Wraps around casacore's random generators to provide complex noise generator with
/// a given variance.
/// @ingroup utils
struct ComplexGaussianNoise 
{
  /// @brief constructor, initializes random generators
  /// @param[in] variance required variance of the noise (same as rms
  /// squared here because the mean is always zero)
  /// @param[in] seed1 a first seed to initialize the random generator
  /// @param[in] seed2 a second seed to initialize the random generator 
  explicit ComplexGaussianNoise(double variance, casa::Int seed1 = 0, 
                                            casa::Int seed2 = 10);

  /// @brief main method to obtain a random complex number.
  /// @details It runs the generator twice for real and imaginary part,
  /// composes a complex number and returns it.
  /// @return a random complex number
  casa::Complex operator()() const;
  
private:
  /// @brief random number generator
  mutable casa::MLCG itsGen;
  /// @brief random number distrubiton
  mutable casa::Normal itsNoiseGen;
}; // ComplexGaussianNoise

} // namespace scimath

} // namespace askap

#endif // #ifndef ASKAP_SCIMATH_UTILS_COMPLEX_GAUSSIAN_NOISE_H

