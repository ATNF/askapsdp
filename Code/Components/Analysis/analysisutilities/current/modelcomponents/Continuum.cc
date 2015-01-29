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
    Spectrum(),
    itsAlpha(0),
    itsBeta(0),
    itsNuZero(defaultFreq)
{
}

Continuum::Continuum(const Spectrum &s):
    Spectrum(s),
    itsAlpha(0),
    itsBeta(0),
    itsNuZero(defaultFreq)
{
}

Continuum::Continuum(const std::string &line, const float nuZero):
    itsNuZero(nuZero)
{
    this->define(line);
}

Continuum::Continuum(const float alpha, const float beta, const float nuZero):
    Spectrum(),
    itsAlpha(alpha),
    itsBeta(beta),
    itsNuZero(nuZero)
{
}

Continuum::Continuum(const float alpha,
                     const float beta,
                     const float nuZero,
                     const float fluxZero):
    Spectrum(),
    itsAlpha(alpha),
    itsBeta(beta),
    itsNuZero(nuZero)
{
    setFluxZero(fluxZero);
}


void Continuum::define(const std::string &line)
{

    std::stringstream ss(line);
    ss >> itsRA >> itsDec >> itsFlux
       >> itsAlpha >> itsBeta
       >> itsMaj >> itsMin >> itsPA;
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
    itsAlpha      = c.itsAlpha;
    itsBeta       = c.itsBeta;
    itsNuZero     = c.itsNuZero;
    return *this;
}

Continuum& Continuum::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Spectrum &) *this) = c;
    this->defineSource(0., 0., defaultFreq);
    return *this;
}

void Continuum::print(std::ostream& theStream)
{
    theStream << itsRA << "\t" << itsDec << "\t"
              << itsFlux << "\t" << itsAlpha << "\t" << itsBeta << "\t"
              << itsMaj << "\t" << itsMin << "\t" << itsPA << "\n";
}

std::ostream& operator<< (std::ostream& theStream, Continuum &cont)
{
    cont.print(theStream);
    return theStream;
}

void Continuum::defineSource(const float alpha, const float beta, const float nuZero)
{
    itsAlpha = alpha;
    itsBeta = beta;
    itsNuZero = nuZero;
}


const double Continuum::flux(const double freq, const int istokes)
{
    if (istokes > 0) {
        return 0.;
    } else {
        double powerTerm = itsAlpha + itsBeta * log(freq / itsNuZero);
        return this->fluxZero() * pow(freq / itsNuZero, powerTerm);
    }
}

const double Continuum::fluxInt(const double freq1, const double freq2, const int istokes)
{
    if (istokes > 0) {
        return 0.;
    } else {
        if (fabs(itsBeta) > 0) {
            ASKAPLOG_ERROR_STR(logger, "Cannot yet integrate with non-zero curvature.");
        }

        double powerTerm = itsAlpha;

        double flux = this->fluxZero() * (pow(std::max(freq1, freq2), powerTerm + 1) -
                                          pow(std::min(freq1, freq2), powerTerm + 1)) /
                      ((powerTerm + 1) * pow(itsNuZero, powerTerm));

        return flux;
    }
}

}


}
