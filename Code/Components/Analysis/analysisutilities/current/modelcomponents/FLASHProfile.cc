/// @file
///
/// Base functions for spectral-line profile classes
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

#include <modelcomponents/FLASHProfile.h>
#include <modelcomponents/GaussianProfile.h>
#include <coordutils/SpectralUtilities.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <scimath/Functionals/Gaussian1D.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".flashprofile");


namespace askap {

namespace analysisutilities {


FLASHProfile::FLASHProfile():
    GaussianProfile()
{
}

FLASHProfile::FLASHProfile(const float restfreq):
    GaussianProfile(restfreq)
{
}

FLASHProfile::FLASHProfile(const double &height,
                           const double &centre,
                           const double &width,
                           const AXISTYPE &type):
    GaussianProfile(height, centre, width, type)
{
}

FLASHProfile::FLASHProfile(const std::string &line, const float restfreq)
{
    itsRestFreq = restfreq;
    this->define(line);
}

FLASHProfile::FLASHProfile(const FLASHProfile& h):
    GaussianProfile(h)
{
    operator=(h);
}

FLASHProfile& FLASHProfile::operator= (const FLASHProfile& h)
{
    if (this == &h) return *this;

    ((GaussianProfile &) *this) = h;
    itsContinuumFlux = h.itsContinuumFlux;
    itsPeakOpticalDepth = h.itsPeakOpticalDepth;
    itsCentreRedshift = h.itsCentreRedshift;
    itsVelocityWidth = h.itsVelocityWidth;
    return *this;
}

void FLASHProfile::define(const std::string &line)
{

    std::stringstream ss(line);
    ss >> itsComponentNum >> itsRA >> itsDec
       >> itsContinuumFlux >> itsMaj >> itsMin >> itsPA
       >> itsPeakOpticalDepth >> itsCentreRedshift >> itsVelocityWidth;
    itsFlux = itsContinuumFlux;
    this->checkShape();
    std::stringstream idstring;
    idstring << itsComponentNum;
    itsID = idstring.str();

    // ASKAPLOG_DEBUG_STR(logger, "FLASH input: " << line);
    // ASKAPLOG_DEBUG_STR(logger, "Defined source " << itsComponentNum << " with continuum flux="<<itsContinuumFlux<<", Component: " << itsComponent << " and Gaussian " << itsGaussian);

}

void FLASHProfile::prepareForUse()
{

    double depth = (exp(-1.*itsPeakOpticalDepth) - 1.) * itsContinuumFlux;
    itsGaussian.setHeight(depth);

    double centreFreq = redshiftToFreq(itsCentreRedshift, itsRestFreq);
    itsGaussian.setCenter(centreFreq);

    double zAsVel = redshiftToVel(itsCentreRedshift);

    double freqmax = velToFreq(zAsVel - itsVelocityWidth / 2., itsRestFreq);
    double freqmin = velToFreq(zAsVel + itsVelocityWidth / 2., itsRestFreq);
    itsGaussian.setWidth(fabs(freqmax - freqmin));

    this->setFreqLimits();

}


void FLASHProfile::print(std::ostream& theStream) const
{
    theStream << itsComponentNum << "\t" << itsRA << "\t" << itsDec << "\t"
              << itsContinuumFlux << "\t"
              << itsMaj << "\t" << itsMin << "\t" << itsPA << "\t"
              << itsPeakOpticalDepth << "\t" << itsCentreRedshift << "\t"
              << itsVelocityWidth << "\n";
}

std::ostream& operator<< (std::ostream& theStream, const FLASHProfile &prof)
{

    prof.print(theStream);
    return theStream;
}

}

}
