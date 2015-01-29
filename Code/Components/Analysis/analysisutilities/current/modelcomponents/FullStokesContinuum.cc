/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_analysisutilities.h>

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumS3SEX.h>
#include <modelcomponents/FullStokesContinuum.h>

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

ASKAP_LOGGER(logger, ".fullstokescontinuum");

namespace askap {

namespace analysisutilities {

FullStokesContinuum::FullStokesContinuum():
    ContinuumS3SEX()
{
    this->defineSource(0., 0., POLREFFREQ);
}

FullStokesContinuum::FullStokesContinuum(const ContinuumS3SEX &c):
    ContinuumS3SEX(c)
{
    this->defineSource(0., 0., POLREFFREQ);
}

FullStokesContinuum::FullStokesContinuum(const Continuum &c):
    ContinuumS3SEX(c)
{
    this->defineSource(0., 0., POLREFFREQ);
}

FullStokesContinuum::FullStokesContinuum(const Spectrum &s):
    ContinuumS3SEX(s)
{
    this->defineSource(0., 0., POLREFFREQ);
}

FullStokesContinuum::FullStokesContinuum(const std::string &line, const float nuZero)
{
    setNuZero(nuZero);
    this->define(line);
}

void FullStokesContinuum::define(const std::string &line)
{

    std::stringstream ss(line);
    ss >> itsComponentNum >> itsClusterID >> itsGalaxyNum
       >> itsSFtype >> itsAGNtype >> itsStructure
       >> itsRA >> itsDec >> itsDistance >> itsRedshift
       >> itsPA >> itsMaj >> itsInputMin
       >> itsI151 >> itsI610 >> itsStokesIref
       >> itsStokesQref >> itsStokesUref >> itsPolFluxRef >> itsPolFracRef
       >> itsI4860 >> itsI18000 >> itsCosVA >> itsRM >> itsRMflag;

    std::stringstream idstring;
    idstring << itsComponentNum;
    itsID = idstring.str();

    if (itsStructure == 4) itsMin = itsMaj * itsCosVA;
    else itsMin = itsInputMin;

    itsFreqValues = std::vector<float>(5);
    for (int i = 0; i < 5; i++) itsFreqValues[i] = freqValuesS3SEX[i];
    itsFreqValues[2] = POLREFFREQ;

    itsI1400 = log10(itsStokesIref);
// Set the reference flux here, but should properly call prepareForUse() to get it right.
    itsFlux = itsStokesIref;
    // ASKAPLOG_DEBUG_STR(logger, "Full Stokes S3SEX object, with flux1400="<<flux1420<<" and itsI1400="<<itsI1400);
    this->checkShape();

    //    ASKAPLOG_DEBUG_STR(logger, "POSSUM source #" << itsComponentNum<<": " << itsRA << " " << itsDec << " " << itsI1400 << " " << itsMaj << " " << itsMin << " " << itsPA);

    itsStokesRefFreq = POLREFFREQ;
    itsStokesVref = 0.;     // Setting Stokes V to be zero for now!
    if (itsPolFluxRef > 0.)
        itsPolAngleRef = 0.5 * acos(itsStokesQref / itsPolFluxRef);
    else
        itsPolAngleRef = 0.;
//      itsAlpha = (log10(itsFlux)-itsI610)/log10(1400./610.);
}

void FullStokesContinuum::print(std::ostream &theStream) const
{
    theStream.setf(std::ios::showpoint);
    theStream << itsComponentNum << std::setw(7) << itsClusterID
              << std::setw(11) << itsGalaxyNum
              << std::setw(3) << itsSFtype << std::setw(3) << itsAGNtype
              << std::setw(3) << itsStructure;
    theStream << std::setw(12) << itsRA << std::setw(12) << itsDec;
    theStream.setf(std::ios::fixed); theStream.unsetf(std::ios::scientific);
    theStream << std::setprecision(3) << std::setw(11) << itsDistance
              << std::setprecision(6) << std::setw(11) << itsRedshift;
    theStream.precision(3);
    theStream << std::setw(10) << itsPA
              << std::setw(10) << itsMaj
              << std::setw(10) << itsInputMin;
    theStream.precision(4);
    theStream << std::setw(10) << itsI151 << std::setw(10) << itsI610;
    theStream.setf(std::ios::scientific); theStream.unsetf(std::ios::fixed);
    theStream << std::setw(12) << itsStokesIref
              << std::setw(12) << itsStokesQref
              << std::setw(12) << itsStokesUref
              << std::setw(12) << itsPolFluxRef;
    theStream.setf(std::ios::fixed); theStream.unsetf(std::ios::scientific);
    theStream << std::setw(10) << itsPolFracRef
              << std::setw(10) << itsI4860
              << std::setw(10) << itsI18000
              << std::setw(10) << itsCosVA
              << std::setw(11) << itsRM
              << std::setw(11) << itsRMflag;
    theStream << "\n";
}

std::ostream& operator<<(std::ostream &theStream, const FullStokesContinuum &stokes)
{
    stokes.print(theStream);
    return theStream;
}


FullStokesContinuum::FullStokesContinuum(const FullStokesContinuum& c):
    ContinuumS3SEX(c)
{
    operator=(c);
}

FullStokesContinuum& FullStokesContinuum::operator= (const FullStokesContinuum& c)
{
    if (this == &c) return *this;

    ((ContinuumS3SEX &) *this) = c;
    itsClusterID = c.itsClusterID;
    itsSFtype = c.itsSFtype;
    itsAGNtype = c.itsAGNtype;
    itsDistance = c.itsDistance;
    itsRedshift = c.itsRedshift;
    itsCosVA = c.itsCosVA;
    itsInputMin = c.itsInputMin;
    itsStokesRefFreq = c.itsStokesRefFreq;
    itsStokesQref = c.itsStokesQref;
    itsStokesUref = c.itsStokesUref;
    itsStokesVref = c.itsStokesVref;
    itsPolFluxRef = c.itsPolFluxRef;
    itsPolFracRef = c.itsPolFracRef;
    itsPolAngleRef = c.itsPolAngleRef;
    itsRM = c.itsRM;
    itsRMflag = c.itsRMflag;
    return *this;
}

FullStokesContinuum& FullStokesContinuum::operator= (const Continuum& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    this->defineSource(0., 0., POLREFFREQ);
    return *this;
}

FullStokesContinuum& FullStokesContinuum::operator= (const ContinuumS3SEX& c)
{
    if (this == &c) return *this;

    ((ContinuumS3SEX &) *this) = c;
    this->defineSource(0., 0., POLREFFREQ);
    return *this;
}

FullStokesContinuum& FullStokesContinuum::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Spectrum &) *this) = c;
    this->defineSource(0., 0., POLREFFREQ);
    return *this;
}

const double FullStokesContinuum::flux(const double freq, const int istokes)
{

    double lambda2, lambdaRef2, angle = 0., flux;
    if (istokes > 0) {
        lambda2 = C * C / (freq * freq);
        lambdaRef2 = C * C / (itsStokesRefFreq * itsStokesRefFreq);
        angle = (lambda2 - lambdaRef2) * itsRM;
    }

    double stokesIFlux =  this->Continuum::flux(freq);
    double polFlux = stokesIFlux * itsPolFracRef; // Assume constant fractional polarisation

    switch (istokes) {
        case 0: // Stokes I
            flux = stokesIFlux;
            break;
        case 1: // Stokes Q
            flux = polFlux * cos(2. * (itsPolAngleRef + angle));
            break;
        case 2: // Stokes U
            flux = polFlux * sin(2. * (itsPolAngleRef + angle));
            break;
        case 3: // Stokes V
            flux = 0.;   // Setting stokes V to zero!
            break;
        default:
            ASKAPLOG_ERROR_STR(logger,
                               "The istokes parameter provided (" <<
                               istokes << ") needs to be in [0,3]");
            flux = 0.;
            break;
    }
    return flux;

}


}

}
