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

#include <modelcomponents/HIprofile.h>
#include <coordutils/SpectralUtilities.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofile");

namespace askap {

namespace analysisutilities {

HIprofile::HIprofile():
    Spectrum(),
    itsRedshift(0.),
    itsMHI(0.),
    itsMinFreq(0.),
    itsMaxFreq(0.)
{
}

HIprofile::HIprofile(const HIprofile& h):
    Spectrum(h)
{
    operator=(h);
}

HIprofile& HIprofile::operator= (const HIprofile& h)
{
    if (this == &h) return *this;

    ((Spectrum &) *this) = h;
    itsRedshift = h.itsRedshift;
    itsMHI = h.itsMHI;
    itsMinFreq = h.itsMinFreq;
    itsMaxFreq = h.itsMaxFreq;
    return *this;
}

const bool HIprofile::freqRangeOK(const double freq1, const double freq2)
{
    double lowfreq = std::min(freq1, freq2);
    double highfreq = std::max(freq1, freq2);
    return (lowfreq < itsMaxFreq) && (highfreq > itsMinFreq);
}

const double HIprofile::integratedFlux(const double z, const double mhi)
{

    itsRedshift = z;
    itsMHI = mhi;
    double dist = redshiftToDist(z); // in Mpc
    double intFlux = 4.24e-6 * mhi / (dist * dist);
    return intFlux;
}

std::ostream& operator<< (std::ostream& theStream, const HIprofile &prof)
{

    theStream << "HI profile summary:\n";
    theStream << "z=" << prof.itsRedshift << "\n";
    theStream << "M_HI=" << prof.itsMHI << "\n";
    return theStream;
}

}

}
