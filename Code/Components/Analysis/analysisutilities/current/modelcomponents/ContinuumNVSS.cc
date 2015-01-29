/// @file
///
/// Continuum source from the NVSS catalogue, using the full content
/// as obtained from CDS, with ascii text/plain option
///
/// @copyright (c) 2010 CSIRO
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
#include <modelcomponents/ContinuumNVSS.h>

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

ASKAP_LOGGER(logger, ".ContNVSS");

namespace askap {

namespace analysisutilities {

ContinuumNVSS::ContinuumNVSS():
    Continuum()
{
    this->defineSource(0., 0., 1400.);
}

ContinuumNVSS::ContinuumNVSS(const Spectrum &s):
    Continuum(s)
{
    this->defineSource(0., 0., 1400.);
}

ContinuumNVSS::ContinuumNVSS(const std::string &line, const float nuZero)
{
    setNuZero(nuZero);
    this->define(line);
}

void ContinuumNVSS::define(const std::string &line)
{
    itsInputLine      = line;
    itsRadius         = atof(line.substr(0, 9).c_str());
    itsXoff           = atof(line.substr(9, 10).c_str());
    itsYoff           = atof(line.substr(19, 10).c_str());
    itsRecno          = atoi(line.substr(30, 8).c_str());
    itsField          = line.substr(38, 8);
    itsFieldXpos      = atof(line.substr(47, 7).c_str());
    itsFieldYpos      = atof(line.substr(55, 7).c_str());
    itsName           = line.substr(63, 14);
    itsRAstring       = line.substr(78, 11);
    itsDecstring      = line.substr(90, 11);
    itsRA_err         = atof(line.substr(102, 5).c_str());
    itsDec_err        = atof(line.substr(107, 4).c_str());
    itsS1400          = atof(line.substr(113, 8).c_str());
    itsS1400_err      = atof(line.substr(122, 7).c_str());
    itsMajorAxisLimit = line[130];
    itsMajorAxis      = atof(line.substr(132, 5).c_str());
    itsMinorAxisLimit = line[138];
    itsMinorAxis      = atof(line.substr(140, 5).c_str());
    itsPA_input       = atof(line.substr(146, 5).c_str());
    itsMajorAxis_err  = atof(line.substr(152, 4).c_str());
    itsMinorAxis_err  = atof(line.substr(157, 4).c_str());
    itsPA_err         = atof(line.substr(161, 4).c_str());
    itsFlagResidual   = line.substr(167, 2);
    itsResidualFlux   = atoi(line.substr(170, 4).c_str());
    itsPolFlux        = atof(line.substr(175, 6).c_str());
    itsPolPA          = atof(line.substr(182, 5).c_str());
    itsPolFlux_err    = atof(line.substr(188, 5).c_str());
    itsPolPA_err      = atof(line.substr(194, 4).c_str());

    itsRA = itsRAstring;
    for (size_t i = 0; i < itsRA.size(); i++) {
        if (itsRA[i] == ' ') {
            itsRA[i] = ':';
        }
    }
    itsDec = itsDecstring;
    for (size_t i = 0; i < itsDec.size(); i++) {
        if (itsDec[i] == ' ') {
            itsDec[i] = ':';
        }
    }

    itsID = itsName;

    itsFlux = itsS1400 / 1.e3; // put into Jy
    itsMaj = itsMajorAxisLimit == '<' ? 0. : itsMajorAxis;
    itsMin = itsMinorAxisLimit == '<' ? 0. : itsMinorAxis;
    itsPA = itsPA_input;

    itsAlpha = 0.;
    itsBeta = 0.;

    this->checkShape();

}

ContinuumNVSS::ContinuumNVSS(const ContinuumNVSS& c):
    Continuum(c)
{
    operator=(c);
}

ContinuumNVSS& ContinuumNVSS::operator= (const ContinuumNVSS& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    itsAlpha      = c.itsAlpha;
    itsBeta       = c.itsBeta;
    itsNuZero     = c.itsNuZero;
    return *this;
}

ContinuumNVSS& ContinuumNVSS::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    this->defineSource(0., 0., 1400.);
    return *this;
}


void ContinuumNVSS::print(std::ostream& theStream) const
{
    theStream << itsInputLine << "\n";
    // theStream.setf(std::ios::showpoint);
    // theStream << std::setw(11) << itsComponentNum << " "
    //    << std::setw(9) << itsGalaxyNum << " "
    //    << std::setw(9)  << itsStructure << " "
    //    << std::setw(15) << std::setprecision(5) << itsRA << " "
    //    << std::setw(11) << std::setprecision(5) << itsDec << " "
    //    << std::setw(14) << std::setprecision(3) << itsPA() << " "
    //    << std::setw(10) << std::setprecision(3) << itsMaj() << " "
    //    << std::setw(10) << std::setprecision(3) << itsMin() << " "
    //    << std::setw(7) << std::setprecision(4) << itsI151 << " "
    //    << std::setw(7) << std::setprecision(4) << itsI610 << " "
    //    << std::setw(7) << std::setprecision(4) << itsI1400 << " "
    //    << std::setw(7) << std::setprecision(4) << itsI4860 << " "
    //    << std::setw(7) << std::setprecision(4) << itsI18000 << "\n";
}
std::ostream& operator<< (std::ostream& theStream, const ContinuumNVSS &cont)
{
    cont.print(theStream);
    return theStream;
}

void ContinuumNVSS::printDetails(std::ostream& theStream) const
{
    theStream << "radius = " << itsRadius
              << "\nXoff = " << itsXoff
              << "\nYoff = " << itsYoff
              << "\nRecno = " << itsRecno
              << "\nField = " << itsField
              << "\nXpos = " << itsFieldXpos
              << "\nYpos = " << itsFieldYpos
              << "\nName = " << itsName
              << "\nRA = " << itsRAstring << " +- " << itsRA_err
              << "\nDec = " << itsDecstring << " +- " << itsDec_err
              << "\nFlux = " << itsS1400 << " +- " << itsS1400_err
              << "\nMajor axis = " << itsMajorAxisLimit << " "
              << itsMajorAxis << " +- " << itsMajorAxis_err
              << "\nMinor axis = " << itsMinorAxisLimit << " "
              << itsMinorAxis << " +- " << itsMinorAxis_err
              << "\nPA = " << itsPA << " +- " << itsPA_err
              << "\nResidual = " << itsFlagResidual << " " << itsResidualFlux
              << "\nPol flux = " << itsPolFlux << " +- " << itsPolFlux_err
              << "\nPol PA = " << itsPolPA << " +- " << itsPolPA_err
              << "\n"
              << "\nRA = " << itsRA
              << "\nDec = " << itsDec
              << "\n";
}

}


}
