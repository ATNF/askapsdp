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
#include <mathsutils/MathsUtils.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <duchamp/duchamp.hh>
#include <duchamp/FitsIO/Beam.hh>
#include <duchamp/Utils/Statistics.hh>
#include <gsl/gsl_sf_gamma.h>
#include <casa/namespace.h>
#include <scimath/Functionals/Gaussian2D.h>

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
    float v1, v2, s;

    // simulate a standard normal RV via polar method
    do {
        v1 = 2.*(1.*random()) / (RAND_MAX + 1.0) - 1.;
        v2 = 2.*(1.*random()) / (RAND_MAX + 1.0) - 1.;
        s = v1 * v1 + v2 * v2;
    } while (s > 1);

    float z = sqrt(-2.*log(s) / s) * v1;
    return z * sigma + mean;
}

double probToZvalue(double prob)
{

    double z = 0, deltaz = 0.1, tolerance = 1.e-6;
    if (prob > 0.5) deltaz *= -1;
    double initial = 0.5 * erfc(z / M_SQRT2) - prob;
    do {
        z += deltaz;
        double current = 0.5 * erfc(z / M_SQRT2) - prob;
        if ((initial * current) < 0) {
            z -= deltaz;
            deltaz /= 10.;
        }
    } while (fabs(deltaz) > tolerance);
    return z;
}


double atanCircular(double sinTerm, double cosTerm)
{
    double epsilon = 1.e-10;
    double angle;
    if (fabs(cosTerm) < epsilon) {
        if (fabs(sinTerm) < epsilon) angle = 0.;
        else if (sinTerm > 0) angle = M_PI / 2.;
        else angle = 3.*M_PI / 2.;
    } else if (fabs(sinTerm) < epsilon) {
        if (cosTerm > 0) angle = 0.;
        else angle = M_PI;
    } else {

        angle = atan(fabs(sinTerm / cosTerm));

        // atan of the absolute value of the ratio returns a value
        // between 0 and 90 degrees.  Need to correct the value of pa
        // according to the correct quandrant it is in.  This is
        // worked out using the signs of sin and cos
        if (sinTerm > 0) {
            if (cosTerm > 0) angle = angle;
            else             angle = M_PI - angle;
        } else {
            if (cosTerm > 0) angle = 2.*M_PI - angle;
            else             angle = M_PI + angle;
        }

    }

    angle = fmod(angle, 2 * M_PI);

    return angle;

}


void findEllipseLimits(double major, double minor, double pa, float &xmin, float &xmax, float &ymin, float &ymax)
{
    double cospa = cos(pa);
    double sinpa = sin(pa);
    double tanpa = tan(pa);
    double x1, x2, y1, y2;
    if (fabs(cospa) < 1.e-8) {
        x1 = -minor;
        x2 = minor;
        y1 = -major;
        y2 = major;
    } else if (fabs(sinpa) < 1.e-8) {
        x1 = -major;
        x2 = major;
        y1 = -minor;
        y2 = minor;
    } else {
        // double t_x1 = atan( tanpa * minor/major);
        double t_x1 = atanCircular(tanpa * minor, major);
        double t_x2 = t_x1 + M_PI;
        // double t_y1 = atan( minor/(major*tanpa));
        double t_y1 = atanCircular(minor, major * tanpa);
        double t_y2 = t_y1 + M_PI;
        x1 = major * cospa * cos(t_x1) - minor * sinpa * sin(t_x1);
        x2 = major * cospa * cos(t_x2) - minor * sinpa * sin(t_x2);
        y1 = major * cospa * cos(t_y1) - minor * sinpa * sin(t_y1);
        y2 = major * cospa * cos(t_y2) - minor * sinpa * sin(t_y2);
    }
    xmin = std::min(x1, x2);
    xmax = std::max(x1, x2);
    ymin = std::min(y1, y2);
    ymax = std::max(y1, y2);

}

double findMiddle(bool robust, std::vector<float> &array)
{
    double middle = 0.;
    size_t size = array.size();

    if (robust) {
        std::vector<float> arrayCopy(array);

        bool isEven = ((size % 2) == 0);
        std::nth_element(arrayCopy.begin(),
                         arrayCopy.begin() + size / 2,
                         arrayCopy.end());
        middle = arrayCopy[size / 2];

        if (isEven) {
            std::nth_element(arrayCopy.begin(),
                             arrayCopy.begin() + size / 2 - 1,
                             arrayCopy.end());
            middle += arrayCopy[size / 2 - 1];
            middle /= 2.;
        }

    } else {
        for (size_t i = 0; i < size; i++) {
            middle += array[i];
        }
        middle /= double(size);
    }

    return middle;

}

double findSpread(bool robust, std::vector<float> &array)
{
    double middle = findMiddle(robust, array);
    return findSpread(robust, middle, array);
}

double findSpread(bool robust, double middle, std::vector<float> &array)
{
    double spread = 0.;
    size_t size = array.size();

    if (robust) {
        std::vector<float> arrayCopy(size, 0);

        for (size_t i = 0; i < size; i++) {
            arrayCopy[i] = fabs(array[i] - middle);
        }

        bool isEven = ((size % 2) == 0);
        std::nth_element(arrayCopy.begin(),
                         arrayCopy.begin() + size / 2,
                         arrayCopy.end());
        spread = arrayCopy[size / 2];

        if (isEven) {
            std::nth_element(arrayCopy.begin(),
                             arrayCopy.begin() + size / 2 - 1,
                             arrayCopy.end());
            spread += arrayCopy[size / 2 - 1];
            spread /= 2.;
        }

        spread = Statistics::madfmToSigma(spread);

    } else {
        for (size_t i = 0; i < size; i++) {
            spread += (array[i] - middle) * (array[i] - middle);
        }
        spread = sqrt(spread / double(size - 1));
    }

    return spread;
}

double findMiddle(bool robust, std::vector<float> &array, std::vector<bool> &mask)
{
    double middle = 0.;
    size_t size = array.size(), goodSize = 0;

    for (size_t i = 0; i < size; i++) {
        if (mask[i]) {
            goodSize++;
        }
    };

    if (robust) {
        std::vector<float> arrayCopy(goodSize, 0.);
        int j = 0;
        for (size_t i = 0; i < size; i++) {
            if (mask[i]) {
                arrayCopy[j++] = array[i];
            }
        }

        bool isEven = ((goodSize % 2) == 0);
        std::nth_element(arrayCopy.begin(),
                         arrayCopy.begin() + goodSize / 2,
                         arrayCopy.end());
        middle = arrayCopy[goodSize / 2];

        if (isEven) {
            std::nth_element(arrayCopy.begin(),
                             arrayCopy.begin() + goodSize / 2 - 1,
                             arrayCopy.end());
            middle += arrayCopy[goodSize / 2 - 1];
            middle /= 2.;
        }

    } else {
        for (size_t i = 0; i < size; i++) {
            if (mask[i]) {
                middle += array[i];
            }
        }
        middle /= double(goodSize);
    }

    return middle;

}

double findSpread(bool robust, std::vector<float> &array, std::vector<bool> &mask)
{
    double middle = findMiddle(robust, array, mask);
    return findSpread(robust, middle, array, mask);
}

double findSpread(bool robust, double middle, std::vector<float> &array, std::vector<bool> &mask)
{

    size_t size = array.size(), goodSize = 0;

    for (size_t i = 0; i < size; i++) {
        if (mask[i]) {
            goodSize++;
        }
    }

    double spread = 0.;

    if (robust) {
        std::vector<float> arrayCopy(goodSize, 0);
        int j = 0;

        for (size_t i = 0; i < size; i++) {
            if (mask[i]) {
                arrayCopy[j++] = fabs(array[i] - middle);
            }
        }

        bool isEven = ((goodSize % 2) == 0);
        std::nth_element(arrayCopy.begin(),
                         arrayCopy.begin() + goodSize / 2,
                         arrayCopy.end());
        spread = arrayCopy[goodSize / 2];

        if (isEven) {
            std::nth_element(arrayCopy.begin(),
                             arrayCopy.begin() + goodSize / 2 - 1,
                             arrayCopy.end());
            spread += arrayCopy[goodSize / 2 - 1];
            spread /= 2.;
        }

        spread = Statistics::madfmToSigma(spread);

    } else {
        for (size_t i = 0; i < size; i++) {
            if (mask[i]) {
                spread += (array[i] - middle) * (array[i] - middle);
            }
        }
        spread = sqrt(spread / double(goodSize - 1));
    }

    return spread;
}

float chisqProb(float ndof, float chisq)
{
    return gsl_sf_gamma_inc(ndof / 2., chisq / 2.) / gsl_sf_gamma(ndof / 2.);
}

std::vector<Double> deconvolveGaussian(casa::Gaussian2D<Double> measured, duchamp::Beam beam)
{
    double a2 = beam.maj(), b2 = beam.min(), pa2 = beam.pa() * M_PI / 180.;
    double a0 = measured.majorAxis(), b0 = measured.minorAxis(), pa0 = measured.PA();
    double twopa0 = 2.0 * pa0, twopa2 = 2.0 * pa2;
    double a0sq = a0 * a0;
    double b0sq = b0 * b0;
    double a2sq = a2 * a2;
    double b2sq = b2 * b2;
    double d0 = a0sq - b0sq;
    double d2 = a2sq - b2sq;

    double d1 = sqrt(d0 * d0 + d2 * d2 - 2.0 * d0 * d2 * cos(twopa0 - twopa2));
    double absum0 = a0sq + b0sq;
    double absum2 = a2sq + b2sq;
    double a1sq = 0.5 * (absum0 - absum2 + d1);
    double b1sq = 0.5 * (absum0 - absum2 - d1);
    double a1 = 0., b1 = 0.;
    if (a1sq > 0.) a1 = sqrt(a1sq);
    if (b1sq > 0.) b1 = sqrt(b1sq);

    double pa1;
    if ((d0 * cos(twopa0) - d2 * cos(twopa2)) == 0.) pa1 = 0.;
    else {
        double sin2pa1 = (d0 * sin(twopa0) - d2 * sin(twopa2));
        double cos2pa1 = (d0 * cos(twopa0) - d2 * cos(twopa2));
        pa1 = atan2(sin2pa1, cos2pa1) / 2.;
    }

    std::vector<Double> deconv(3);
    double maj = std::max(std::max(a1, b1), 0.);
    double min = std::max(std::min(a1, b1), 0.);
    deconv[0] = maj;
    deconv[1] = min;
    deconv[2] = pa1;

    return deconv;

}


}

}

