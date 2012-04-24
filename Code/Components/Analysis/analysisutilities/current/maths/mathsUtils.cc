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
#include <askap_analysisutilities.h>

#include <maths/mathsUtils.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".mathsutils");

namespace askap {

    namespace analysisutilities {


        float normalRandomVariable(float mean, float sigma)
        {
            /// @details Simulate a normal random variable from a
            /// distribution with mean given by mean and standard deviation
            /// given by sigma. The variable is simulated via the polar
            /// method.
            /// @param mean The mean of the normal distribution
            /// @param sigma The standard deviation of the normal distribution
            /// @return A random variable.
            float v1, v2, s;

            // simulate a standard normal RV via polar method
            do {
                v1 = 2.*(1.*random()) / (RAND_MAX + 1.0) - 1.;
                v2 = 2.*(1.*random()) / (RAND_MAX + 1.0) - 1.;
                s = v1 * v1 + v2 * v2;
            } while (s > 1);

            float z = sqrt(-2.*log(s) / s) * v1;
            return z*sigma + mean;
        }

      double atanCircular(double sinTerm, double cosTerm)
      {
	double epsilon=1.e-10;
	double angle;
	if(fabs(cosTerm) < epsilon){
	  if(fabs(sinTerm)<epsilon) angle=0.;
	  else if(sinTerm > 0) angle=M_PI/2.;
	  else angle=3.*M_PI/2.;
	}
	else if(fabs(sinTerm)<epsilon){
	  if(cosTerm>0) angle=0.;
	  else angle=M_PI;
	}
	else{

	  angle = atan(fabs(sinTerm / cosTerm));
	  
	  // atan of the absolute value of the ratio returns a value between 0 and 90 degrees.
	  // Need to correct the value of pa according to the correct quandrant it is in.
	  // This is worked out using the signs of sin and cos
	  if (sinTerm > 0) {
	    if (cosTerm > 0) angle = angle;
	    else             angle = M_PI - angle;
	  } else {
	    if (cosTerm > 0) angle = 2.*M_PI - angle;
	    else             angle = M_PI + angle;
	  }
	  
	}

	angle=fmod(angle,2*M_PI);

	return angle;

      }


      void findEllipseLimits(double major, double minor, double pa, float &xmin, float &xmax, float &ymin, float &ymax)
      {

	// Use the parametric equation for an ellipse (u = a cos(t), v = b sin(t)) to find the limits of x and y once converted from u & v.

	double cospa = cos(pa);
	double sinpa = sin(pa);
	double tanpa = tan(pa);
	double x1,x2,y1,y2;
	if(fabs(cospa)<1.e-8) {
	  x1=-minor;
	  x2=minor;
	  y1=-major;
	  y2=major;
	}
	else if(fabs(sinpa)<1.e-8){
	  x1=-major;
	  x2=major;
	  y1=-minor;
	  y2=minor;
	}
	else{
	  // double t_x1 = atan( tanpa * minor/major);
	  double t_x1 = atanCircular(tanpa*minor,major);
	  double t_x2 = t_x1 + M_PI;
	  // double t_y1 = atan( minor/(major*tanpa));
	  double t_y1 = atanCircular(minor, major*tanpa);
	  double t_y2 = t_y1 + M_PI;
	  x1=major*cospa*cos(t_x1) - minor*sinpa*sin(t_x1);
	  x2=major*cospa*cos(t_x2) - minor*sinpa*sin(t_x2);
	  y1=major*cospa*cos(t_y1) - minor*sinpa*sin(t_y1);
	  y2=major*cospa*cos(t_y2) - minor*sinpa*sin(t_y2);
	}
	xmin=std::min(x1,x2);
	xmax=std::max(x1,x2);
	ymin=std::min(y1,y2);
	ymax=std::max(y1,y2);

      }


        double findSpread(bool robust, double middle, int size, float *array)
        {
            /// @details
            /// Finds the "spread" (ie. the rms or standard deviation) of an
            /// array of values using a given mean value. The option exists
            /// to use the standard deviation, or, by setting robust=true,
            /// the median absolute deviation from the median. In the latter
            /// case, the middle value given is assumed to be the median,
            /// and the returned value is the median absolute difference of
            /// the data values from the median.
            double spread = 0.;

            if (robust) {
                float *arrayCopy = new float[size];

                for (int i = 0; i < size; i++) arrayCopy[i] = fabs(array[i] - middle);

                bool isEven = ((size % 2) == 0);
                std::nth_element(arrayCopy, arrayCopy + size / 2, arrayCopy + size);
                spread = arrayCopy[size/2];

                if (isEven) {
                    std::nth_element(arrayCopy, arrayCopy + size / 2 - 1, arrayCopy + size);
                    spread += arrayCopy[size/2-1];
                    spread /= 2.;
                }

                delete [] arrayCopy;
                spread = Statistics::madfmToSigma(spread);
            } else {
                for (int i = 0; i < size; i++) spread += (array[i] - middle) * (array[i] - middle);

                spread = sqrt(spread / double(size - 1));
            }

            return spread;
        }


        double findSpread(bool robust, double middle, int size, float *array, bool *mask)
        {
            /// @details
            /// Finds the "spread" (ie. the rms or standard deviation) of an
            /// array of values using a given mean value. The option exists
            /// to use the standard deviation, or, by setting robust=true,
            /// the median absolute deviation from the median. In the latter
            /// case, the middle value given is assumed to be the median,
            /// and the returned value is the median absolute difference of
            /// the data values from the median.
            int goodSize = 0;

            for (int i = 0; i < size; i++) if (mask[i]) goodSize++;

            double spread = 0.;

            if (robust) {
                float *arrayCopy = new float[goodSize];
                int j = 0;

                for (int i = 0; i < size; i++) if (mask[i]) arrayCopy[j++] = fabs(array[i] - middle);

                bool isEven = ((goodSize % 2) == 0);
                std::nth_element(arrayCopy, arrayCopy + goodSize / 2, arrayCopy + goodSize);
                spread = arrayCopy[goodSize/2];

                if (isEven) {
                    std::nth_element(arrayCopy, arrayCopy + goodSize / 2 - 1, arrayCopy + goodSize);
                    spread += arrayCopy[goodSize/2-1];
                    spread /= 2.;
                }

                delete [] arrayCopy;
                spread = Statistics::madfmToSigma(spread);
            } else {
                for (int i = 0; i < size; i++) if (mask[i]) spread += (array[i] - middle) * (array[i] - middle);

                spread = sqrt(spread / double(goodSize - 1));
            }

            return spread;
        }

        float chisqProb(float ndof, float chisq)
        {
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
            return gsl_sf_gamma_inc(ndof / 2., chisq / 2.) / gsl_sf_gamma(ndof / 2.);
        }

      std::vector<Double> deconvolveGaussian(casa::Gaussian2D<Double> measured, duchamp::Beam beam)
      {
	/// @details Deconvolution of a Gaussian shape, assuming it
	/// was convolved with the given beam. This procedure
	/// replicates the approach described in Wild (1970), AuJPh
	/// 23, 113.
	/// @param measured Gaussian shape to be deconvolved
	/// @param beam Beam shape of image
	/// @return A vector containing (in order), the major & minor
	/// axes, and the position angle (in radians).
	double a2=beam.maj(),b2=beam.min(),pa2=beam.pa()*M_PI/180.;
	double a0=measured.majorAxis(),b0=measured.minorAxis(),pa0=measured.PA();
	// ASKAPLOG_DEBUG_STR(logger, "About to deconvolve Gaussian of size " << a0 << "x"<<b0<<"_"<<pa0*180./M_PI <<" from beam "<<a2 << "x"<<b2<<"_"<<pa2*180./M_PI);
	double d0=a0*a0-b0*b0,d2=a2*a2-b2*b2;

	double d1 = sqrt( d0*d0 + d2*d2 - 2*d0*d2*cos(2.*(pa0-pa2)) );
	double a1sq = 0.5*(a0*a0+b0*b0-a2*a2-b2*b2+d1), b1sq = 0.5*(a0*a0+b0*b0-a2*a2-b2*b2-d1);
	double a1=0.,b1=0.;
	if(a1sq>0.) a1=sqrt(a1sq);
	if(b1sq>0.) b1=sqrt(b1sq);
	// ASKAPLOG_DEBUG_STR(logger, "Deconvolving: d0="<<d0<<", d2="<<d2<<", d1="<<d1<<", a1sq="<<a1sq<<", b1sq="<<b1sq<<", a1="<<a1<<", b1="<<b1);
	double pa1;
	if((d0*cos(2.*pa0)-d2*cos(2.*pa2))==0.) pa1=0.;
	else{
	  double sin2pa1 = (d0 * sin(2.*pa0) + d2 * sin(2.*pa2));
	  double cos2pa1 = (d0 * cos(2.*pa0) + d2 * cos(2.*pa2));
	  pa1 = atanCircular(sin2pa1, cos2pa1);
	//   pa1 = atan(fabs(sin2pa1 / cos2pa1));

	//   // atan of the absolute value of the ratio returns a value between 0 and 90 degrees.
	//   // Need to correct the value of pa according to the correct quandrant it is in.
	//   // This is worked out using the signs of sin and cos
	//   if (sin2pa1 > 0) {
	//     if (cos2pa1 > 0) pa1 = pa1;
	//     else             pa1 = M_PI - pa1;
	//   } else {
	//     if (cos2pa1 > 0) pa1 = 2.*M_PI - pa1;
	//     else             pa1 = M_PI + pa1;
	//   }
	// }

	std::vector<Double> deconv(3);
	double maj=std::max(std::max(a1,b1),0.);
	double min=std::max(std::min(a1,b1),0.);
	// ASKAPLOG_DEBUG_STR(logger, "Deconvolved sizes: a1="<<a1<<", b1="<<b1<<",  maj="<<maj<<", min="<<min<<", pa1="<<pa1);
	deconv[0] = maj;
	deconv[1] = min;
	deconv[2] = pa1;

	return deconv;

      }


    }

}
