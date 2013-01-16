/// @file
///
/// Provides utility functions for simulations package
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSISUTILS_MATHS_H_
#define ASKAP_ANALYSISUTILS_MATHS_H_

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <casa/namespace.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <duchamp/FitsIO/Beam.hh>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

    namespace analysisutilities {

      /// @brief Convert between a Gaussian's sigma and its FWHM
      inline double FWHMtoSIGMA(double f){return f / (2. * M_SQRT2 * sqrt(M_LN2));};
      inline double SIGMAtoFWHM(double s){return s * (2. * M_SQRT2 * sqrt(M_LN2));};
      
      /// @brief Return a normal random variable
      float normalRandomVariable(float mean, float rms);
      
      double atanCircular(double sinTerm, double cosTerm);
      
      void findEllipseLimits(double major, double minor, double pa, float &xmin, float &xmax, float &ymin, float &ymax);

      /// @brief Find an rms for an array given a mean value
      /// @ingroup analysisutilities
      double findSpread(bool robust, double middle, int size, float *array);
      
      /// @brief Find an rms for an array given a mean value, with masking of pixels.
      /// @ingroup analysisutilities
      double findSpread(bool robust, double middle, int size, float *array, bool *mask);

      /// @brief Return the probability of obtaining a chisq value by
      ///        chance, for a certain number of degrees of freedom.
      /// @ingroup analysisutilities
      float chisqProb(float ndof, float chisq);

      /// @brief Return the Gaussian after deconvolution with the given beam
      std::vector<Double> deconvolveGaussian(casa::Gaussian2D<Double> measured, duchamp::Beam beam);

    }

}


#endif
