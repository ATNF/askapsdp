/// @file
///
/// Class to manage HI profiles for the SKADS S3-SAX simulation.
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

#include <coordutils/SpectralUtilities.h>
#include <modelcomponents/HIprofile.h>
#include <modelcomponents/HIprofileS3SAX.h>
#include <modelcomponents/HIprofileS3SAX.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <math.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofiles3sax");


namespace askap {

namespace analysisutilities {

HIprofileS3SAX::HIprofileS3SAX(const HIprofileS3SAX & h):
    HIprofile(h)
{
    operator=(h);
}

HIprofileS3SAX& HIprofileS3SAX::operator= (const HIprofileS3SAX & h)
{
    if (this == &h) return *this;

    ((HIprofile &) *this) = h;
    itsFluxPeak = h.itsFluxPeak;
    itsFlux0 = h.itsFlux0;
    itsWidthPeak = h.itsWidthPeak;
    itsWidth50 = h.itsWidth50;
    itsWidth20 = h.itsWidth20;
    itsIntFlux = h.itsIntFlux;
    itsSideFlux = h.itsSideFlux;
    itsMiddleFlux = h.itsMiddleFlux;
    itsKpar = h.itsKpar;
    return *this;
}

void HIprofileS3SAX::diagnostic(std::ostream & theStream) const
{
    theStream << "HI profile summary:\n";
    theStream << "z=" << itsRedshift << "\n";
    theStream << "M_HI=" << itsMHI << "\n";
    theStream << "Fpeak=" << itsFluxPeak << "\n";
    theStream << "F0=" << itsFlux0 << "\n";
    theStream << "Wpeak=" << itsWidthPeak << "\n";
    theStream << "W50=" << itsWidth50 << "\n";
    theStream << "W20=" << itsWidth20 << "\n";
    theStream << "IntFlux=" << itsIntFlux << "\n";
    theStream << "Side Flux=" << itsSideFlux << "\n";
    theStream << "Middle Flux=" << itsMiddleFlux << "\n";
    theStream << "K[] = [" << itsKpar[0];

    for (int i = 1; i < 5; i++)
        theStream << "," << itsKpar[i];

    theStream << "]\n";

    std::pair<double, double> freqRange = this->freqLimits();
    theStream << "Freq Range = " << freqRange.first << " - " << freqRange.second << "\n";

}

void HIprofileS3SAX::print(std::ostream & theStream) const
{
    theStream << itsRA << "\t" << itsDec << "\t" << itsIntFlux << "\t"
              << itsMaj << "\t" << itsMin << "\t" << itsPA << "\t"
              << itsRedshift << "\t" << itsMHI << "\t"
              << itsFlux0 << "\t" << itsFluxPeak << "\t"
              << itsWidthPeak << "\t" << itsWidth50 << "\t"
              << itsWidth20 << "\n";
}

std::ostream& operator<< (std::ostream & theStream, const HIprofileS3SAX & prof)
{

    prof.print(theStream);
    return theStream;
}

HIprofileS3SAX::HIprofileS3SAX(const std::string & line)
{
    this->define(line);
}

void HIprofileS3SAX::define(const std::string & line)
{

    std::stringstream ss(line);
    ss >> itsRA >> itsDec >> itsIntFlux
       >> itsMaj >> itsMin >> itsPA
       >> itsRedshift >> itsMHI
       >> itsFlux0 >> itsFluxPeak
       >> itsWidthPeak >> itsWidth50 >> itsWidth20;
    itsFlux = itsFluxPeak * itsIntFlux;
    this->checkShape();
    this->PosToID();
}

void HIprofileS3SAX::prepareForUse()
{

    const double lnhalf = log(0.5);
    const double lnfifth = log(0.2);
    itsKpar = std::vector<double>(5);
    double a = itsFlux0;
    double b = itsFluxPeak;
    double c = itsWidthPeak;
    double d = itsWidth50;
    double e = itsWidth20;

    itsKpar[0] = 0.25 * (lnhalf * (c * c - e * e) + lnfifth * (d * d - c * c)) /
                 (lnhalf * (c - e) + lnfifth * (d - c));
    itsKpar[1] = (0.25 * (c * c - d * d) + itsKpar[0] * (d - c)) / lnhalf;
    itsKpar[2] = b * exp((2.*itsKpar[0] - c) * (2.*itsKpar[0] - c) /
                         (4.*itsKpar[1]));

    if (fabs(a - b) / a < 1.e-8) {
        itsKpar[3] = itsKpar[4] = 0.;
    } else if (c > 0) {
        itsKpar[3] = c * c * b * b / (4.*(b * b - a * a));
        itsKpar[4] = a * sqrt(itsKpar[3]);
    } else {
        itsKpar[3] = itsKpar[4] = 0.;
    }

    itsSideFlux = (itsKpar[2] * sqrt(itsKpar[1]) / M_2_SQRTPI) *
                  erfc((0.5 * c - itsKpar[0]) / sqrt(itsKpar[1]));

    if (fabs(a - b) / a < 1.e-8) {
        itsMiddleFlux = a * c;
    } else if (c > 0) {
        itsMiddleFlux = 2. * itsKpar[4] *
                        atan(c / sqrt(4.*itsKpar[3] - c * c));
    } else {
        itsMiddleFlux = 0.;
    }

    double maxAbsVel = itsKpar[0] +
                       sqrt(itsKpar[1] * log(itsKpar[2] * MAXFLOAT));
    float vel0 = redshiftToVel(itsRedshift);
    itsMaxFreq = HIVelToFreq(vel0 + maxAbsVel);
    itsMinFreq = HIVelToFreq(vel0 - maxAbsVel);
    if (itsMinFreq > itsMaxFreq) std::swap(itsMinFreq, itsMaxFreq);

}

const double HIprofileS3SAX::flux(const double nu, const int istokes)
{

    if (istokes > 0) return 0.;
    else {
        double flux;
        double dvel = freqToHIVel(nu) - redshiftToVel(itsRedshift);

        if (fabs(dvel) < 0.5 * itsWidthPeak)
            flux = itsKpar[4] / sqrt(itsKpar[3] - dvel * dvel);
        else {
            double temp = fabs(dvel) - itsKpar[0];
            flux = itsKpar[2] * exp(-temp * temp / itsKpar[1]);
        }

        return flux * itsIntFlux;
    }

}


const double HIprofileS3SAX::fluxInt(const double nu1, const double nu2, const int istokes)
{

    if (istokes > 0) return 0.;
    else {
        double f[2], dv[2];
        double a = itsFlux0, b = itsFluxPeak, c = itsWidthPeak;
        // lowest relative velocity:
        dv[0] = freqToHIVel(std::max(nu1, nu2)) - redshiftToVel(itsRedshift);
        // highest relative velocity:
        dv[1] = freqToHIVel(std::min(nu1, nu2)) - redshiftToVel(itsRedshift);
        f[0] = f[1] = 0.;

        for (int i = 0; i < 2; i++) {
            if (dv[i] < -0.5 * c) {
                f[i] += (itsKpar[2] * sqrt(itsKpar[1]) / M_2_SQRTPI) *
                        erfc((0. - dv[i] - itsKpar[0]) / sqrt(itsKpar[1]));
            } else {
                f[i] += itsSideFlux;

                if (dv[i] < 0.5 * c) {
                    if (fabs(a - b) / a < 1.e-8) {
                        f[i] += a * (dv[i] + 0.5 * c);
                    } else {
                        f[i] += itsKpar[4] *
                                (atan(dv[i] / sqrt(itsKpar[3] - dv[i] * dv[i])) +
                                 atan(c / sqrt(4.*itsKpar[3] - c * c)));
                    }
                } else {
                    f[i] += itsMiddleFlux;
                    f[i] += itsSideFlux -
                            (itsKpar[2] * sqrt(itsKpar[1]) / M_2_SQRTPI) *
                            erfc((dv[i] - itsKpar[0]) / sqrt(itsKpar[1]));
                }
            }

        }

        double flux = (f[1] - f[0]) / (dv[1] - dv[0]);
        return flux * itsIntFlux;
    }
}


const std::pair<double, double> HIprofileS3SAX::freqLimits() const
{

    double maxAbsVel = itsKpar[0] +
                       sqrt(itsKpar[1] * log(itsKpar[2] * MAXFLOAT));
    std::pair<double, double> freqLimits;
    float vel0 = redshiftToVel(itsRedshift);
    freqLimits.first = HIVelToFreq(vel0 + maxAbsVel);
    freqLimits.second = HIVelToFreq(vel0 - maxAbsVel);
    return freqLimits;

}



}

}
