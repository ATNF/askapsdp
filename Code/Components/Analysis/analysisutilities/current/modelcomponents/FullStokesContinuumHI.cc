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

#include <modelcomponents/FullStokesContinuumHI.h>
#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumS3SEX.h>
#include <modelcomponents/HIprofileS3SEX.h>
#include <modelcomponents/FullStokesContinuum.h>

#include <mathsutils/MathsUtils.h>
#include <cosmology/Cosmology.h>

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

ASKAP_LOGGER(logger, ".fullstokescontinuumHI");

namespace askap {

namespace analysisutilities {

const GALTYPE getGaltype(const int sftype, const int agntype)
{
    GALTYPE type = UNKNOWN;
    switch (sftype) {
        case 0:
            switch (agntype) {
                case 0:
                    ASKAPLOG_ERROR_STR(logger, "Both sftype and agntype = 0.");
                    break;
                case 1:
                    type = RQAGN; break;
                case 2:
                    type = FRI; break;
                case 3:
                    type = FRII; break;
                case 4:
                    type = GPS; break;
                default:
                    ASKAPLOG_ERROR_STR(logger, "Unknown value " << agntype << " for agntype");
            };
            break;
        case 1:
            type = SFG; break;
        case 2:
            type = SBG; break;
        default:
            break;
    }

    return type;

}

FullStokesContinuumHI::FullStokesContinuumHI():
    FullStokesContinuum(), itsHIprofile()
{
}

FullStokesContinuumHI::FullStokesContinuumHI(const ContinuumS3SEX &c):
    FullStokesContinuum(c), itsHIprofile()
{
}

FullStokesContinuumHI::FullStokesContinuumHI(const Continuum &c):
    FullStokesContinuum(c), itsHIprofile()
{
}

FullStokesContinuumHI::FullStokesContinuumHI(const Spectrum &s):
    FullStokesContinuum(s), itsHIprofile()
{
}

FullStokesContinuumHI::FullStokesContinuumHI(const std::string &line, const float nuZero)
{
    setNuZero(nuZero);
    this->define(line);
}

void FullStokesContinuumHI::define(const std::string &line)
{

    this->FullStokesContinuum::define(line);

    double HImass = 0.;
    GALTYPE type = getGaltype(itsSFtype, itsAGNtype);
    if (type == SFG || type == SBG) {
        cosmology::Cosmology cosmo;
        double lum = cosmo.lum(itsRedshift, itsI1400 - 26.);
        lum *= M_LN10; // convert to natural log from log_10

        // Want to add some dispersion to HImass, a la Wilman et al
        // Calculate the delta by converting the component number
        // (modulo 1000) to a probability, then interpret that as a
        // normal prob
        double prob = (itsComponentNum % 1000 + 0.5) / 1000.;
        double z = probToZvalue(prob);
        HImass = 0.44 * lum  + 0.48 + z * 0.3;

//      ASKAPLOG_DEBUG_STR(logger, "HI profile for component #"<< itsComponentNum << " gives a prob of " << prob << " and a z-value of " << z << " giving a delta-M of " << 0.3*z << " and log10(M_HI)="<<log10(exp(HImass)));

        HImass = exp(HImass);
//      ASKAPLOG_DEBUG_STR(logger, "Creating HI profile with M_HI = " << HImass<<", using log10(flux)="<<itsI1400<<" to get a lum of " << lum);
    }

    itsHIprofile = HIprofileS3SEX(type, itsRedshift, HImass, itsMaj, itsMin, itsComponentNum, itsGalaxyNum);
//    itsHIprofile.diagnostic(std::cout);

}

void FullStokesContinuumHI::print(std::ostream &theStream) const
{

    theStream.setf(std::ios::showpoint);
    theStream << itsComponentNum << std::setw(7) << itsClusterID
              << std::setw(11) << itsGalaxyNum
              << std::setw(3) << itsSFtype
              << std::setw(3) << itsAGNtype
              << std::setw(3) << itsStructure;
    theStream << std::setw(12) << itsRA << std::setw(12) << itsDec;
    theStream.setf(std::ios::fixed); theStream.unsetf(std::ios::scientific);
    theStream << std::setprecision(3) << std::setw(11) << itsDistance
              << std::setprecision(6) << std::setw(11) << itsRedshift;
    theStream.precision(3);
    theStream << std::setw(10) << itsPA
              << std::setw(10) << itsMaj
              << std::setw(10) << itsMin;
    theStream.precision(4);
    theStream << std::setw(10) << itsI151
              << std::setw(10) << itsI610;
    theStream.setf(std::ios::scientific); theStream.unsetf(std::ios::fixed);
    theStream << std::setw(12) << itsFlux
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
    theStream.setf(std::ios::scientific); theStream.unsetf(std::ios::fixed);
    theStream << std::setw(13) << std::setprecision(6) << itsHIprofile.mHI();
    theStream << "\n";
}

std::ostream& operator<<(std::ostream &theStream, const FullStokesContinuumHI &stokes)
{
    stokes.print(theStream);
    return theStream;
}


FullStokesContinuumHI::FullStokesContinuumHI(const FullStokesContinuumHI& c):
    FullStokesContinuum(c)
{
    operator=(c);
}

FullStokesContinuumHI& FullStokesContinuumHI::operator= (const FullStokesContinuumHI& c)
{
    if (this == &c) return *this;

    ((FullStokesContinuum &) *this) = c;
    itsHIprofile = c.itsHIprofile;
    return *this;
}

FullStokesContinuumHI& FullStokesContinuumHI::operator= (const Continuum& c)
{
    if (this == &c) return *this;

    ((Continuum &) *this) = c;
    this->defineSource(0., 0., POLREFFREQ);
    return *this;
}

FullStokesContinuumHI& FullStokesContinuumHI::operator= (const ContinuumS3SEX& c)
{
    if (this == &c) return *this;

    ((ContinuumS3SEX &) *this) = c;
    this->defineSource(0., 0., POLREFFREQ);
    return *this;
}

FullStokesContinuumHI& FullStokesContinuumHI::operator= (const Spectrum& c)
{
    if (this == &c) return *this;

    ((Spectrum &) *this) = c;
    this->defineSource(0., 0., POLREFFREQ);
    return *this;
}

}

}
