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
#include <modelcomponents/ContinuumSelavy.h>

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

ContinuumSelavy::ContinuumSelavy(const bool flagUseDeconvolvedSizes):
    Continuum(),
    itsFlagUseDeconvolvedSizes(flagUseDeconvolvedSizes)
{
}

ContinuumSelavy::ContinuumSelavy(const Spectrum &s, const bool flagUseDeconvolvedSizes):
    Continuum(s),
    itsFlagUseDeconvolvedSizes(flagUseDeconvolvedSizes)
{
}

ContinuumSelavy::ContinuumSelavy(const float alpha,
                                 const float beta,
                                 const float nuZero):
    Continuum(alpha, beta, nuZero),
    itsFlagUseDeconvolvedSizes(defaultDeconvFlag)
{
}

ContinuumSelavy::ContinuumSelavy(const float alpha,
                                 const float beta,
                                 const float nuZero,
                                 const float fluxZero):
    Continuum(alpha, beta, nuZero, fluxZero)
{
}

ContinuumSelavy::ContinuumSelavy(const std::string &line,
                                 const float nuZero,
                                 const bool flagUseDeconvolvedSizes):
    itsFlagUseDeconvolvedSizes(flagUseDeconvolvedSizes)
{
    setNuZero(nuZero);
    this->define(line);
}

void ContinuumSelavy::define(const std::string &line)
{

    std::stringstream ss(line);
    int flag;
    ss >> itsID >> itsName >> itsRA >> itsDec
       >> itsX >> itsY
       >> itsFint >> itsFpeak >> itsFintFIT >> itsFpeakFIT
       >> itsMajFIT >> itsMinFIT >> itsPAFIT
       >> itsMajDECONV >> itsMinDECONV >> itsPADECONV
       >> itsAlpha >> itsBeta
       >> itsChisq >> itsRMSimage >> itsRMSfit
       >> itsNfree >> itsNdof >> itsNpixFIT >> itsNpixObj >> flag;

    if (itsFlagUseDeconvolvedSizes) {
        this->setMaj(std::max(itsMajDECONV, itsMinDECONV));
        this->setMin(std::min(itsMajDECONV, itsMinDECONV));
        this->setPA(itsPADECONV);
    } else {
        this->setMaj(std::max(itsMajFIT, itsMinFIT));
        this->setMin(std::min(itsMajFIT, itsMinFIT));
        this->setPA(itsPAFIT);
    }
    this->setFluxZero(itsFintFIT);
    itsFlagGuess = (flag == 1);

    //ASKAPLOG_DEBUG_STR(logger, "Selavy source #" << itsID <<": " << itsRA << " " << itsDec << " " << itsFintFIT << " " << itsMaj << " " << itsMin << " " << itsPA);

}

ContinuumSelavy::ContinuumSelavy(const ContinuumSelavy& c):
    Continuum(c)
{
    operator=(c);
}

ContinuumSelavy& ContinuumSelavy::operator= (const ContinuumSelavy& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    itsName = c.itsName;
    itsX = c.itsX;
    itsY = c.itsY;
    itsFint = c.itsFint;
    itsFpeak = c.itsFpeak;
    itsFintFIT = c.itsFintFIT;
    itsFpeakFIT = c.itsFpeakFIT;
    itsMajFIT = c.itsMajFIT;
    itsMinFIT = c.itsMinFIT;
    itsPAFIT = c.itsPAFIT;
    itsMajDECONV = c.itsMajDECONV;
    itsMinDECONV = c.itsMinDECONV;
    itsPADECONV = c.itsPADECONV;
    itsChisq = c.itsChisq;
    itsRMSimage = c.itsRMSimage;
    itsRMSfit = c.itsRMSfit;
    itsNfree = c.itsNfree;
    itsNdof = c.itsNdof;
    itsNpixFIT = c.itsNpixFIT;
    itsNpixObj = c.itsNpixObj;
    itsFlagGuess = c.itsFlagGuess;
    itsFlagUseDeconvolvedSizes = c.itsFlagUseDeconvolvedSizes;
    return *this;
}

ContinuumSelavy& ContinuumSelavy::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    itsFlagUseDeconvolvedSizes = false;
    this->defineSource(0., 0., 1400.);
    return *this;
}


void ContinuumSelavy::print(std::ostream& theStream)
{
    theStream.setf(std::ios::showpoint);
    theStream << std::setw(6) << itsID << " "
              << std::setw(14) << itsName << " "
              << std::setw(15) << std::setprecision(5) << itsRA << " "
              << std::setw(11) << std::setprecision(5) << itsDec << " "
              << std::setw(8) << std::setprecision(1) << itsX << " "
              << std::setw(8) << std::setprecision(1) << itsY << " "
              << std::setw(10) << std::setprecision(8) << itsFint << " "
              << std::setw(10) << std::setprecision(8) << itsFpeak << " "
              << std::setw(10) << std::setprecision(8) << itsFintFIT << " "
              << std::setw(10) << std::setprecision(8) << itsFpeakFIT << " "
              << std::setw(8) << std::setprecision(3) << itsMajFIT << " "
              << std::setw(8) << std::setprecision(3) << itsMinFIT << " "
              << std::setw(8) << std::setprecision(3) << itsPAFIT << " "
              << std::setw(8) << std::setprecision(3) << itsMajDECONV << " "
              << std::setw(8) << std::setprecision(3) << itsMinDECONV << " "
              << std::setw(8) << std::setprecision(3) << itsPADECONV << " "
              << std::setw(6) << std::setprecision(3) << itsAlpha << " "
              << std::setw(6) << std::setprecision(3) << itsBeta << " "
              << std::setw(27) << std::setprecision(9) << itsChisq << " "
              << std::setw(10) << std::setprecision(8) << itsRMSimage << " "
              << std::setw(15) << std::setprecision(6) << itsRMSfit << " "
              << std::setw(11) << itsNfree << " "
              << std::setw(10) << itsNdof << " "
              << std::setw(10) << itsNpixFIT << " "
              << std::setw(10) << itsNpixObj << " "
              << std::setw(7)  << itsFlagGuess << "\n";
}
std::ostream& operator<< (std::ostream& theStream, ContinuumSelavy &cont)
{
    cont.print(theStream);
    return theStream;
}
}


}
