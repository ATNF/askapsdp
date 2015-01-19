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

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".continuum");

namespace askap {

namespace analysisutilities {

Continuum::Continuum():
    Spectrum()
{
    this->defineSource(0., 0., 1400.);
}

Continuum::Continuum(Spectrum &s):
    Spectrum(s)
{
    this->defineSource(0., 0., 1400.);
}

Continuum::Continuum(std::string &line)
{
    this->define(line);
}

void Continuum::define(const std::string &line)
{

    std::stringstream ss(line);
    ss >> this->itsRA >> this->itsDec >> this->itsFlux
       >> this->itsAlpha >> this->itsBeta
       >> this->itsMaj >> this->itsMin >> this->itsPA;
    this->PosToID();
    this->checkShape();

}

Continuum::Continuum(const Continuum& c):
    Spectrum(c)
{
    operator=(c);
}

Continuum& Continuum::operator= (const Continuum& c)
{
    if (this == &c) return *this;

    ((Spectrum &) *this) = c;
    this->itsAlpha      = c.itsAlpha;
    this->itsBeta       = c.itsBeta;
    this->itsNuZero     = c.itsNuZero;
    return *this;
}

Continuum& Continuum::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Spectrum &) *this) = c;
    this->defineSource(0., 0., 1400.);
    return *this;
}

void Continuum::print(std::ostream& theStream)
{
    theStream << this->itsRA << "\t" << this->itsDec << "\t"
              << this->itsFlux << "\t" << this->itsAlpha << "\t" << this->itsBeta << "\t"
              << this->itsMaj << "\t" << this->itsMin << "\t" << this->itsPA << "\n";
}

std::ostream& operator<< (std::ostream& theStream, Continuum &cont)
{
    cont.print(theStream);
    return theStream;
}

void Continuum::defineSource(float alpha, float beta, float nuZero)
{
    this->itsAlpha = alpha;
    this->itsBeta = beta;
    this->itsNuZero = nuZero;
}


double Continuum::flux(double freq, int istokes)
{
    if (istokes > 0) return 0.;
    else {
        double powerTerm = this->itsAlpha + this->itsBeta * log(freq / this->itsNuZero);
        return this->fluxZero() * pow(freq / this->itsNuZero, powerTerm);
    }
}

double Continuum::fluxInt(double freq1, double freq2, int istokes)
{
    if (istokes > 0) return 0.;
    else {
        if (fabs(this->itsBeta) > 0) {
            ASKAPLOG_ERROR_STR(logger, "Cannot yet integrate with non-zero curvature.");
        }

        double powerTerm = this->itsAlpha;

        double flux = this->fluxZero() * (pow(std::max(freq1, freq2), powerTerm + 1) -
                                          pow(std::min(freq1, freq2), powerTerm + 1)) /
                      ((powerTerm + 1) * pow(this->itsNuZero, powerTerm));

        return flux;
    }
}

}


}
