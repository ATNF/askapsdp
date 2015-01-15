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
inline double FWHMtoSIGMA(double f) {return f / (2. * M_SQRT2 * sqrt(M_LN2));};
inline double SIGMAtoFWHM(double s) {return s * (2. * M_SQRT2 * sqrt(M_LN2));};

/// @brief Return a normal random variable
/// @details Simulate a normal random variable from a
/// distribution with mean given by mean and standard deviation
/// given by sigma. The variable is simulated via the polar
/// method.
/// @param mean The mean of the normal distribution
/// @param sigma The standard deviation of the normal distribution
/// @return A random variable.
float normalRandomVariable(float mean, float rms);

/// @brief Return the standard normal z-score corresponding to a probability (integrated from -infinity)
double probToZvalue(double prob);

double atanCircular(double sinTerm, double cosTerm);

/// Use the parametric equation for an ellipse (u = a cos(t), v = b
/// sin(t)) to find the limits of x and y once converted from u & v.
void findEllipseLimits(double major, double minor, double pa, float &xmin, float &xmax, float &ymin, float &ymax);

/// Find an rms for an array given a mean value.
/// Finds the "spread" (ie. the rms or standard deviation) of an
/// array of values using a given mean value. The option exists
/// to use the standard deviation, or, by setting robust=true,
/// the median absolute deviation from the median. In the latter
/// case, the middle value given is assumed to be the median,
/// and the returned value is the median absolute difference of
/// the data values from the median.
/// @ingroup analysisutilities
double findSpread(bool robust, double middle, std::vector<float> array);

/// Find an rms for an array given a mean value, with masking of pixels.
/// Finds the "spread" (ie. the rms or standard deviation) of an
/// array of values using a given mean value. The option exists
/// to use the standard deviation, or, by setting robust=true,
/// the median absolute deviation from the median. In the latter
/// case, the middle value given is assumed to be the median,
/// and the returned value is the median absolute difference of
/// the data values from the median.
/// @ingroup analysisutilities
double findSpread(bool robust, double middle, std::vector<float> array, std::vector<bool> mask);

/// @brief Return the probability of obtaining a chisq value by
///        chance, for a certain number of degrees of freedom.
/// @ingroup analysisutilities
float chisqProb(float ndof, float chisq);

/// @brief Return the Gaussian after deconvolution with the given beam
std::vector<Double> deconvolveGaussian(casa::Gaussian2D<Double> measured, duchamp::Beam beam);

}

}


#endif
