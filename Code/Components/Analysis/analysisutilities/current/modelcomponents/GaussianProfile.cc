/// @file
///
/// Base functions for spectral-line profile classes
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <askap_analysisutilities.h>

#include <modelcomponents/GaussianProfile.h>
#include <coordutils/SpectralUtilities.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <scimath/Functionals/Gaussian1D.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".gaussianprofile");


namespace askap {

namespace analysisutilities {


GaussianProfile::GaussianProfile():
    Spectrum(),
    itsAxisType(defaultAxisType),
    itsRestFreq(defaultRestFreq)
{
}

GaussianProfile::GaussianProfile(const float restfreq):
    Spectrum(),
    itsAxisType(defaultAxisType),
    itsRestFreq(restfreq)
{
}

GaussianProfile::GaussianProfile(const double &height,
                                 const double &centre,
                                 const double &width,
                                 const AXISTYPE &type):
    Spectrum(),
    itsGaussian(height, centre, width),
    itsAxisType(type),
    itsRestFreq(defaultRestFreq)
{
}

GaussianProfile::GaussianProfile(const std::string &line, const float restfreq):
    itsAxisType(defaultAxisType),
    itsRestFreq(restfreq)
{
    this->define(line);
}


GaussianProfile::GaussianProfile(const GaussianProfile& h):
    Spectrum(h)
{
    operator=(h);
}

GaussianProfile& GaussianProfile::operator= (const GaussianProfile& h)
{
    if (this == &h) return *this;

    ((Spectrum &) *this) = h;
    itsGaussian = h.itsGaussian;
    itsAxisType = h.itsAxisType;
    itsRestFreq = h.itsRestFreq;
    itsMinFreq = h.itsMinFreq;
    itsMaxFreq = h.itsMaxFreq;
    return *this;
}

void GaussianProfile::define(const std::string &line)
{

    double peak, centre, width;
    std::stringstream ss(line);
    ss >> itsRA >> itsDec >> itsFlux
       >> itsMaj >> itsMin >> itsPA
       >> peak >> centre >> width;

    this->PosToID();
    this->checkShape();
    if (itsMaj < itsMin) std::swap(itsMaj, itsMin);

    itsGaussian.setHeight(peak);
    itsGaussian.setCenter(centre);
    itsGaussian.setWidth(width);

}

void GaussianProfile::setFreqLimits()
{
    double sigma = itsGaussian.width() / (2. * sqrt(2 * M_LN2));
    itsMinFreq = itsGaussian.center() -
                 sigma * sqrt(2.*log(MAXFLOAT * itsGaussian.height()));
    itsMaxFreq = itsGaussian.center() +
                 sigma * sqrt(2.*log(MAXFLOAT * itsGaussian.height()));

    switch (itsAxisType) {
        case PIXEL:
            ASKAPLOG_ERROR_STR(logger, "Cannot use axis type PIXEL");
            break;
        case FREQUENCY:
            if (itsMinFreq > itsMaxFreq) {
                std::swap(itsMinFreq, itsMaxFreq);
            }
            break;
        case VELOCITY:
            itsMinFreq = velToFreq(itsMinFreq, itsRestFreq);
            itsMaxFreq = velToFreq(itsMaxFreq, itsRestFreq);
            if (itsMinFreq > itsMaxFreq) {
                std::swap(itsMinFreq, itsMaxFreq);
            }
            break;
        case REDSHIFT:
            itsMinFreq = redshiftToFreq(itsMinFreq, itsRestFreq);
            itsMaxFreq = redshiftToFreq(itsMaxFreq, itsRestFreq);
            if (itsMinFreq > itsMaxFreq) {
                std::swap(itsMinFreq, itsMaxFreq);
            }
            break;
    }

}

const bool GaussianProfile::freqRangeOK(const double freq1, const double freq2)
{
    double lowfreq = std::min(freq1, freq2);
    double highfreq = std::max(freq1, freq2);
    return (lowfreq < itsMaxFreq) && (highfreq > itsMinFreq);
}

const double GaussianProfile::flux(const double nu, const int istokes)
{
    if (istokes > 0) return 0.;
    else {
        double flux = 0.;
        switch (itsAxisType) {
            case PIXEL:
                ASKAPLOG_ERROR_STR(logger, "Cannot use axis type PIXEL");
                break;
            case FREQUENCY:
                flux = itsGaussian(nu);
                break;
            case VELOCITY:
                flux = itsGaussian(freqToVel(nu, itsRestFreq));
                break;
            case REDSHIFT:
                flux = itsGaussian(freqToRedshift(nu, itsRestFreq));
                break;
        }
        return flux;
    }
}

const double GaussianProfile::fluxInt(const double nu1, const double nu2, const int istokes)
{
    if (istokes > 0) return 0.;
    else {
        double scale, first, last, v1, v2, z1, z2;
        double flux = 0.;
        switch (itsAxisType) {
            case PIXEL:
                ASKAPLOG_ERROR_STR(logger, "Cannot use axis type PIXEL");
                break;
            case FREQUENCY:
                scale = itsGaussian.width() * itsGaussian.height()  /
                        (2.*sqrt(M_LN2)) / M_2_SQRTPI;
                first = (std::min(nu1, nu2) - itsGaussian.center()) *
                        2. * sqrt(M_LN2) / itsGaussian.width();
                last = (std::max(nu1, nu2) - itsGaussian.center()) *
                       2. * sqrt(M_LN2) / itsGaussian.width();
                flux = scale * (erf(last) - erf(first));
                break;
            case VELOCITY:
                v1 = freqToVel(nu1, itsRestFreq);
                v2 = freqToVel(nu2, itsRestFreq);
                scale =  itsGaussian.width() * itsGaussian.height()  /
                         (2.*sqrt(M_LN2)) / M_2_SQRTPI;
                first = (std::min(v1, v2) - itsGaussian.center()) *
                        2. * sqrt(M_LN2) / itsGaussian.width();
                last = (std::max(v1, v2) - itsGaussian.center()) *
                       2. * sqrt(M_LN2) / itsGaussian.width();
                flux = scale * (erf(last) - erf(first));
                break;
            case REDSHIFT:
                z1 = freqToRedshift(nu1, itsRestFreq);
                z2 = freqToRedshift(nu2, itsRestFreq);
                scale =  itsGaussian.width() * itsGaussian.height()  /
                         (2.*sqrt(M_LN2)) / M_2_SQRTPI;
                first = (std::min(z1, z2) - itsGaussian.center()) *
                        2. * sqrt(M_LN2) / itsGaussian.width();
                last = (std::max(z1, z2) - itsGaussian.center()) *
                       2. * sqrt(M_LN2) / itsGaussian.width();
                flux = scale * (erf(last) - erf(first));
                break;
        }
        flux = flux / fabs(nu2 - nu1);
        // ASKAPLOG_DEBUG_STR(logger, "Flux between " << nu1 << " and " << nu2 << " is " << flux << " with scale=" << scale << " and basic integral b/w " << first << "and " << last << " = " << erf(last)-erf(first) );
        return flux ;
    }
}

std::ostream& operator<< (std::ostream& theStream, const GaussianProfile &prof)
{

    theStream << "Gaussian profile summary:\n";
    theStream << prof.itsGaussian << "\n";
    return theStream;
}

}

}
