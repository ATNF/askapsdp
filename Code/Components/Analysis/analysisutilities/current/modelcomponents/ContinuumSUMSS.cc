/// @file
///
/// Continuum source from the SUMSS catalogue, version 2.1, as obtained
/// from http://www.physics.usyd.edu.au/sifa/Main/SUMSS
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
#include <modelcomponents/ContinuumSUMSS.h>

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

ASKAP_LOGGER(logger, ".ContSUMSS");

namespace askap {

namespace analysisutilities {

ContinuumSUMSS::ContinuumSUMSS():
    Continuum()
{
    this->defineSource(0., 0., 1400.);
}

ContinuumSUMSS::ContinuumSUMSS(Spectrum &s):
    Continuum(s)
{
    this->defineSource(0., 0., 1400.);
}

ContinuumSUMSS::ContinuumSUMSS(std::string &line, float nuZero)
{
    this->setNuZero(nuZero);
    this->define(line);
}

void ContinuumSUMSS::define(const std::string &line)
{

    itsInputLine      = line;
    std::stringstream ss(line);
    ss >> itsRAh >> itsRAm >> itsRAs
       >> itsDECd >> itsDECm >> itsDECs
       >> itsRAerr >> itsDECerr
       >> itsPeakFlux >> itsPeakFluxErr
       >> itsTotalFlux >> itsTotalFluxErr
       >> itsFittedMajorAxis >> itsFittedMinorAxis >> itsFittedPositionAngle
       >> itsDeconvMajorAxis >> itsDeconvMinorAxis
       >> itsDeconvPositionAngleString
       >> itsMosaicName >> itsNumMosaics >> itsXpos >> itsYpos;

    itsRA = itsRAh + ":" + itsRAm + ":" + itsRAs;
    itsDec = itsDECd + ":" + itsDECm + ":" + itsDECs;
//    this->PosToID();
    std::stringstream sID;
    sID << "J" << itsRAh << itsRAm << itsDECd << itsDECm;
    itsID = sID.str();

    itsFlux = itsTotalFlux / 1.e3;  //convert to Jy
    itsMaj = itsDeconvMajorAxis;
    itsMin = itsDeconvMinorAxis;
    itsPA = (itsDeconvPositionAngleString == "---") ? 0. :
            atof(itsDeconvPositionAngleString.c_str());

    itsAlpha = 0.;
    itsBeta = 0.;

    this->checkShape();

}

ContinuumSUMSS::ContinuumSUMSS(const ContinuumSUMSS& c):
    Continuum(c)
{
    operator=(c);
}

ContinuumSUMSS& ContinuumSUMSS::operator= (const ContinuumSUMSS& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    itsAlpha      = c.itsAlpha;
    itsBeta       = c.itsBeta;
    itsNuZero     = c.itsNuZero;
    return *this;
}

ContinuumSUMSS& ContinuumSUMSS::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    this->defineSource(0., 0., 1400.);
    return *this;
}


void ContinuumSUMSS::print(std::ostream& theStream)
{
    theStream << itsInputLine << "\n";
}

std::ostream& operator<< (std::ostream& theStream, ContinuumSUMSS &cont)
{

    cont.print(theStream);
    return theStream;
}

}


}
