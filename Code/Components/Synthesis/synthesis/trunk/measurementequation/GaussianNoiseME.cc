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

#include <measurementequation/GaussianNoiseME.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief constructor, initializes random distribution required.
/// @param[in] variance required variance of the noise (same as rms
/// here because the mean is always zero.
/// @param[in] seed1 a first seed to initialize the random generator
/// @param[in] seed2 a second seed to initialize the random generator 
GaussianNoiseME::GaussianNoiseME(double variance, casa::Int seed1, casa::Int seed2) :
  itsGen(seed1, seed2), itsNoiseGen(&itsGen,0.,variance) {}
 
/// @brief Predict model visibilities for one accessor (chunk).
/// @details This prediction is done for single chunk of data only. 
/// It seems that all measurement equations should work with accessors 
/// rather than iterators (i.e. the iteration over chunks should be 
/// moved to the higher level, outside this class). 
/// @param[in] chunk a read-write accessor to work with
void GaussianNoiseME::predict(IDataAccessor &chunk) const
{
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
  for (casa::uInt row = 0; row<rwVis.nrow(); ++row) {
       bool isAutoCorrelation = false;
       if (chunk.antenna1()(row) == chunk.antenna2()(row)) {
           if (chunk.feed1()(row) == chunk.feed2()(row)) {
               isAutoCorrelation = true;
           }
       }
       for (casa::uInt chan = 0; chan<rwVis.ncolumn(); ++chan) {
            for (casa::uInt pol = 0; pol<rwVis.nplane(); ++pol) {
                 if (isAutoCorrelation) {
                     rwVis(row,chan,pol) = std::abs(getRandomComplexNumber());
                 } else {
                     rwVis(row,chan,pol) = getRandomComplexNumber();
                 }
            }
       }
  }
}

/// @brief Calculate the normal equation for one accessor (chunk).
/// @details This calculation is done for a single chunk of
/// data only (one iteration).It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). 
/// @param[in] chunk a read-write accessor to work with
/// @param[in] ne Normal equations
void GaussianNoiseME::calcEquations(const IConstDataAccessor &chunk,
                          askap::scimath::INormalEquations& ne) const
{
  ASKAPTHROW(AskapError, "GaussianNoiseME::calcEquations can not be called. There is probably a logical error.");
}

/// @brief a helper method to obtain a random complex number.
/// @details It runs the generator twice for real and imaginary part,
/// composes a complex number and returns it.
/// @return a random complex number
casa::Complex GaussianNoiseME::getRandomComplexNumber() const
{
  return casa::Complex(itsNoiseGen(),itsNoiseGen());
}
