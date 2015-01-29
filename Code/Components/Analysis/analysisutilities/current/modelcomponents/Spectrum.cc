/// @file
///
/// Base class functions for spectral profiles.
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

#include <iostream>
#include <iomanip>

#include <modelcomponents/Spectrum.h>
#include <coordutils/PositionUtilities.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".spectrum");


namespace askap {

namespace analysisutilities {

Spectrum::Spectrum(const std::string &line)
{
    this->define(line);
}

void Spectrum::define(const std::string &line)
{

    std::stringstream ss(line);
    ss >> itsRA >> itsDec >> itsFlux >>
       itsMaj >> itsMin >> itsPA;
    this->PosToID();
    this->checkShape();
}

void Spectrum::PosToID()
{
    itsID = itsRA + "_" + itsDec;
}

void Spectrum::checkShape()
{
    if (itsMaj < itsMin) {
        float t = itsMaj;
        itsMaj = itsMin;
        itsMin = t;
    }
}


Spectrum::Spectrum(const Spectrum& s)
{
    itsID = s.itsID;
    itsRA = s.itsRA;
    itsDec = s.itsDec;
    itsFlux = s.itsFlux;
    itsMaj = s.itsMaj;
    itsMin = s.itsMin;
    itsPA = s.itsPA;
}

void Spectrum::setRA(const double r, const int prec)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(prec) << r;
    itsRA = ss.str();
}

void Spectrum::setDec(const double d, const int prec)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(prec) << d;
    itsDec = ss.str();
}

const double Spectrum::raD()
{
    return raToDouble(itsRA);
}

const double Spectrum::decD()
{
    return decToDouble(itsDec);
}


void Spectrum::print(std::ostream& theStream, const std::string ra, const std::string dec)
{
    std::string oldRA = itsRA;
    std::string oldDec = itsDec;
    itsRA = ra;
    itsDec = dec;
    this->print(theStream);
    itsRA = oldRA;
    itsDec = oldDec;
}

void Spectrum::print(std::ostream& theStream, const double ra, const double dec, const int prec)
{
    std::string oldRA = itsRA;
    std::string oldDec = itsDec;
    this->setRA(ra);
    this->setDec(dec);
    this->print(theStream);
    itsRA = oldRA;
    itsDec = oldDec;
}
void Spectrum::print(std::ostream& theStream) const
{
    theStream << itsRA << "\t" << itsDec << "\t" << itsFlux << "\t"
              << itsMaj << "\t" << itsMin << "\t" << itsPA << "\n";
}

std::ostream& operator<< (std::ostream& theStream, const Spectrum &spec)
{
    spec.print(theStream);
    return theStream;
}



}

}
