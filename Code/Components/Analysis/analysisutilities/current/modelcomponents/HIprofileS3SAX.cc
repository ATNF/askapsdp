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
        this->itsFluxPeak = h.itsFluxPeak;
        this->itsFlux0 = h.itsFlux0;
        this->itsWidthPeak = h.itsWidthPeak;
        this->itsWidth50 = h.itsWidth50;
        this->itsWidth20 = h.itsWidth20;
        this->itsIntFlux = h.itsIntFlux;
        this->itsSideFlux = h.itsSideFlux;
        this->itsMiddleFlux = h.itsMiddleFlux;
        this->itsKpar = h.itsKpar;
        return *this;
    }

    void HIprofileS3SAX::diagnostic(std::ostream & theStream)
    {
        theStream << "HI profile summary:\n";
        theStream << "z=" << this->itsRedshift << "\n";
        theStream << "M_HI=" << this->itsMHI << "\n";
        theStream << "Fpeak=" << this->itsFluxPeak << "\n";
        theStream << "F0=" << this->itsFlux0 << "\n";
        theStream << "Wpeak=" << this->itsWidthPeak << "\n";
        theStream << "W50=" << this->itsWidth50 << "\n";
        theStream << "W20=" << this->itsWidth20 << "\n";
        theStream << "IntFlux=" << this->itsIntFlux << "\n";
        theStream << "Side Flux=" << this->itsSideFlux << "\n";
        theStream << "Middle Flux=" << this->itsMiddleFlux << "\n";
        theStream << "K[] = [" << this->itsKpar[0];

        for (int i = 1; i < 5; i++)
            theStream << "," << this->itsKpar[i];

        theStream << "]\n";

        std::pair<double, double> freqRange = this->freqLimits();
        theStream << "Freq Range = " << freqRange.first << " - " << freqRange.second << "\n";

    }

    void HIprofileS3SAX::print(std::ostream & theStream)
    {
        theStream << this->itsRA << "\t" << this->itsDec << "\t" << this->itsIntFlux << "\t"
        << this->itsMaj << "\t" << this->itsMin << "\t" << this->itsPA << "\t"
        << this->itsRedshift << "\t" << this->itsMHI << "\t"
        << this->itsFlux0 << "\t" << this->itsFluxPeak << "\t"
        << this->itsWidthPeak << "\t" << this->itsWidth50 << "\t"
        << this->itsWidth20 << "\n";
    }

    std::ostream& operator<< (std::ostream & theStream, HIprofileS3SAX & prof)
    {

        prof.print(theStream);
        return theStream;
    }

    HIprofileS3SAX::HIprofileS3SAX(std::string & line)
    {
        this->define(line);
    }

    void HIprofileS3SAX::define(const std::string & line)
    {

        std::stringstream ss(line);
        ss >> this->itsRA >> this->itsDec >> this->itsIntFlux
        >> this->itsMaj >> this->itsMin >> this->itsPA
        >> this->itsRedshift >> this->itsMHI
        >> this->itsFlux0 >> this->itsFluxPeak
        >> this->itsWidthPeak >> this->itsWidth50 >> this->itsWidth20;
        this->itsFlux = this->itsFluxPeak * this->itsIntFlux;
        this->checkShape();
        this->PosToID();
    }

    void HIprofileS3SAX::prepareForUse()
    {

        const double lnhalf = log(0.5);
        const double lnfifth = log(0.2);
        this->itsKpar = std::vector<double>(5);
        double a = this->itsFlux0;
        double b = this->itsFluxPeak;
        double c = this->itsWidthPeak;
        double d = this->itsWidth50;
        double e = this->itsWidth20;

        this->itsKpar[0] = 0.25 * (lnhalf * (c * c - e * e) + lnfifth * (d * d - c * c)) /
        (lnhalf * (c - e) + lnfifth * (d - c));
        this->itsKpar[1] = (0.25 * (c * c - d * d) + this->itsKpar[0] * (d - c)) / lnhalf;
        this->itsKpar[2] = b * exp((2.*this->itsKpar[0] - c) * (2.*this->itsKpar[0] - c) /
        (4.*this->itsKpar[1]));

        if (fabs(a - b) / a < 1.e-8) {
            this->itsKpar[3] = this->itsKpar[4] = 0.;
        } else if (c > 0) {
            this->itsKpar[3] = c * c * b * b / (4.*(b * b - a * a));
            this->itsKpar[4] = a * sqrt(this->itsKpar[3]);
        } else {
            this->itsKpar[3] = this->itsKpar[4] = 0.;
        }

        this->itsSideFlux = (this->itsKpar[2] * sqrt(this->itsKpar[1]) / M_2_SQRTPI) *
                            erfc((0.5 * c - this->itsKpar[0]) / sqrt(this->itsKpar[1]));

        if (fabs(a - b) / a < 1.e-8) {
            this->itsMiddleFlux = a * c;
        } else if (c > 0) {
            this->itsMiddleFlux = 2. * this->itsKpar[4] *
                                  atan(c / sqrt(4.*this->itsKpar[3] - c * c));
        } else {
            this->itsMiddleFlux = 0.;
        }

        double maxAbsVel = this->itsKpar[0] +
                           sqrt(this->itsKpar[1] * log(this->itsKpar[2] * MAXFLOAT));
        float vel0 = redshiftToVel(this->itsRedshift);
        this->itsMaxFreq = HIVelToFreq(vel0 + maxAbsVel);
        this->itsMinFreq = HIVelToFreq(vel0 - maxAbsVel);
        if (this->itsMinFreq > this->itsMaxFreq) std::swap(this->itsMinFreq, this->itsMaxFreq);

    }

    double HIprofileS3SAX::flux(double nu, int istokes)
    {

        if (istokes > 0) return 0.;
        else {
            double flux;
            double dvel = freqToHIVel(nu) - redshiftToVel(this->itsRedshift);

            if (fabs(dvel) < 0.5 * this->itsWidthPeak)
                flux = this->itsKpar[4] / sqrt(this->itsKpar[3] - dvel * dvel);
            else {
                double temp = fabs(dvel) - this->itsKpar[0];
                flux = this->itsKpar[2] * exp(-temp * temp / this->itsKpar[1]);
            }

            return flux * this->itsIntFlux;
        }

    }


    double HIprofileS3SAX::fluxInt(double nu1, double nu2, int istokes)
    {

        if (istokes > 0) return 0.;
        else {
            double f[2], dv[2];
            double a = this->itsFlux0, b = this->itsFluxPeak, c = this->itsWidthPeak;
            // lowest relative velocity:
            dv[0] = freqToHIVel(std::max(nu1, nu2)) - redshiftToVel(this->itsRedshift);
            // highest relative velocity:
            dv[1] = freqToHIVel(std::min(nu1, nu2)) - redshiftToVel(this->itsRedshift);
            f[0] = f[1] = 0.;

            for (int i = 0; i < 2; i++) {
                if (dv[i] < -0.5 * c) {
                    f[i] += (this->itsKpar[2] * sqrt(this->itsKpar[1]) / M_2_SQRTPI) *
                            erfc((0. - dv[i] - this->itsKpar[0]) / sqrt(this->itsKpar[1]));
                } else {
                    f[i] += this->itsSideFlux;

                    if (dv[i] < 0.5 * c) {
                        if (fabs(a - b) / a < 1.e-8) {
                            f[i] += a * (dv[i] + 0.5 * c);
                        } else {
                            f[i] += this->itsKpar[4] *
                                    (atan(dv[i] / sqrt(this->itsKpar[3] - dv[i] * dv[i])) +
                                     atan(c / sqrt(4.*this->itsKpar[3] - c * c)));
                        }
                    } else {
                        f[i] += this->itsMiddleFlux;
                        f[i] += this->itsSideFlux -
                                (this->itsKpar[2] * sqrt(this->itsKpar[1]) / M_2_SQRTPI) *
                                erfc((dv[i] - this->itsKpar[0]) / sqrt(this->itsKpar[1]));
                    }
                }

            }

            double flux = (f[1] - f[0]) / (dv[1] - dv[0]);
            return flux * this->itsIntFlux;
        }
    }


    std::pair<double, double> HIprofileS3SAX::freqLimits()
    {

        double maxAbsVel = this->itsKpar[0] +
                           sqrt(this->itsKpar[1] * log(this->itsKpar[2] * MAXFLOAT));
        std::pair<double, double> freqLimits;
        float vel0 = redshiftToVel(this->itsRedshift);
        freqLimits.first = HIVelToFreq(vel0 + maxAbsVel);
        freqLimits.second = HIVelToFreq(vel0 - maxAbsVel);
        return freqLimits;

    }



}

}
