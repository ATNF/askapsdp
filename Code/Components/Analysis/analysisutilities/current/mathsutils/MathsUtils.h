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
inline const double FWHMtoSIGMA(const double f) {return f / (2. * M_SQRT2 * sqrt(M_LN2));};
inline const double SIGMAtoFWHM(const double s) {return s * (2. * M_SQRT2 * sqrt(M_LN2));};

/// @brief Return a normal random variable
/// @details Simulate a normal random variable from a
/// distribution with mean given by mean and standard deviation
/// given by sigma. The variable is simulated via the polar
/// method.
/// @param mean The mean of the normal distribution
/// @param sigma The standard deviation of the normal distribution
/// @return A random variable.
const float normalRandomVariable(const float mean, const float rms);

/// @brief Return the standard normal z-score corresponding to a probability (integrated from -infinity)
const double probToZvalue(const double prob);

const double atanCircular(const double sinTerm, const double cosTerm);

/// Use the parametric equation for an ellipse (u = a cos(t), v = b
/// sin(t)) to find the limits of x and y once converted from u & v.
void findEllipseLimits(const double major, const double minor, const double pa,
                       float &xmin, float &xmax, float &ymin, float &ymax);

/// Find the mean or robust estimate thereof for an array.  Finds the
/// "middle" (ie. the mean) of an array of values. The option exists
/// to use the mean, or, by setting robust=true, the median.
const double findMiddle(const bool robust,
                        const std::vector<float> &array);

/// Find an rms or robust estimate thereof for an array given a mean value.
/// Finds the "spread" (ie. the rms or standard deviation) of an
/// array of values using a given mean value. The option exists
/// to use the standard deviation, or, by setting robust=true,
/// the median absolute deviation from the median. In the latter
/// case, the middle value given is assumed to be the median,
/// and the returned value is the median absolute difference of
/// the data values from the median.
/// @ingroup analysisutilities
const double findSpread(const bool robust,
                        const double middle,
                        const std::vector<float> &array);

/// Find an rms robust estimate thereof for an array.  Calls
/// findMiddle first, then uses that value to call
/// findSpread(bool,double,std::vector<float>&).
const double findSpread(const bool robust,
                        const std::vector<float> &array);

/// Find the mean or robust estimate thereof for an array, with
/// masking of pixels.  Finds the "middle" (ie. the mean) of an array
/// of values. The option exists to use the mean, or, by setting
/// robust=true, the median. Only pixels where the mask is true are
/// used.
const double findMiddle(const bool robust,
                        const std::vector<float> &array,
                        const std::vector<bool> &mask);

/// Find an rms for an array given a mean value, with masking of
/// pixels.  Finds the "spread" (ie. the rms or standard deviation) of
/// an array of values using a given mean value. The option exists to
/// use the standard deviation, or, by setting robust=true, the median
/// absolute deviation from the median. In the latter case, the middle
/// value given is assumed to be the median, and the returned value is
/// the median absolute difference of the data values from the
/// median. Only pixels where the mask is true are used.
const double findSpread(const bool robust,
                        const double middle,
                        const std::vector<float> &array,
                        const std::vector<bool> &mask);

/// Find an rms robust estimate thereof for an array, with masking of
/// pixels.  Calls findMiddle first, then uses that value to call
/// findSpread(bool,double,std::vector<float>&,std::vector<bool>&).
const double findSpread(const bool robust,
                        const std::vector<float> &array,
                        const std::vector<bool> &mask);

/// @brief Return the probability of obtaining a chisq value by
/// chance, for a certain number of degrees of freedom.
/// @ingroup analysisutilities
/// @details Returns the probability of exceeding the given
/// value of chisq by chance. If it comes from a fit, this
/// probability is assuming the fit is valid.
///
/// Typical use: say you have a fit with ndof=5 degrees of
/// freedom that gives a chisq value of 12. You call this
/// function via chisqProb(5,12.), which will return
/// 0.0347878. If your confidence limit is 95% (ie. you can
/// tolerate a 1-in-20 chance that a valid fit will produce a
/// chisq value that high), you would reject that fit (since
/// 0.0347878 < 0.05), but if it is 99%, you would accept it
/// (since 0.0347878 > 0.01).
const float chisqProb(const float ndof, const float chisq);

/// @brief Return the Gaussian after deconvolution with the given beam
/// @details Deconvolution of a Gaussian shape, assuming it
/// was convolved with the given beam. This procedure
/// replicates the approach described in Wild (1970), AuJPh
/// 23, 113.
/// @param measured Gaussian shape to be deconvolved
/// @param beam Beam shape of image
/// @return A vector containing (in order), the major & minor
/// axes, and the position angle (in radians).
const std::vector<Double>
deconvolveGaussian(const casa::Gaussian2D<Double> &measured, duchamp::Beam beam);

}

}


#endif
