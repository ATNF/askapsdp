/// @file
///
/// XXX Notes on program XXX
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <askap_analysisutilities.h>

#include <coordutils/SpectralUtilities.h>
#include <cosmology/Cosmology.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".spectrautils");


namespace askap {

namespace analysisutilities {

using namespace cosmology;

const double redshiftToDist(const double z, const Cosmology cosmology)
{
    /// @details Converts redshift to a distance. Currently just a
    /// front-end to the Cosmology::dlum() function.
    /// @param z The redshift
    /// @param cosmology The set of cosmological parameters to use.
    /// @return The luminosity distance in Mpc

    return cosmology.dlum(z);
}

const double redshiftToVel(const double z)
{
    /// @details Converts a redshift to a recessional velocity,
    /// using the relativistic equation.
    /// @param z The redshift
    /// @return The corresponding velocity in km/s

    double v = ((z + 1.) * (z + 1.) - 1.) / ((z + 1.) * (z + 1.) + 1.);
    return C_kms * v;
}

const double velToRedshift(const double vel)
{
    /// @details Converts a recessional velocity to a redshift
    /// using the relativistic equation.
    /// @param z The redshift
    /// @return The corresponding velocity in km/s

    double v = vel / C_kms;
    double z = sqrt((1. + v) / (1. - v)) - 1.;
    return z;
}

const double redshiftToHIFreq(const double z)
{
    /// @details Converts a redshift to the observed frequency of an HI line.
    /// @param z The redshift
    /// @return The corresponding frequency in Hz

    return redshiftToFreq(z, nu0_HI);
}

const double redshiftToFreq(const double z, const double restfreq)
{
    /// @details Converts a redshift to the observed frequency of a spectral line with rest frequency as specified.
    /// @param z The redshift
    /// @param restfreq The rest frequency of the line
    /// @return The corresponding frequency, in the same units as the rest frequency.

    return restfreq / (z + 1);
}


const double HIFreqToRedshift(const double freq)
{
    /// @details Converts the observed frequency of an HI line to a redshift.
    /// @param freq The frequency in Hz
    /// @return The corresponding redshift

    return freqToRedshift(freq, nu0_HI);
}

const double freqToRedshift(const double freq, const double restfreq)
{
    /// @details Convertsthe observed frequency of a spectral line, with rest frequency as specified, to a redshift. Note both frequencies need to be in the same units
    /// @param freq The frequency
    /// @param restfreq The rest frequency of the line
    /// @return The corresponding redshift

    return (restfreq / freq) - 1.;
}


const double freqToVel(const double nu, const double restfreq)
{
    /// @details Converts a frequency to the velocity of a line with given rest frequency
    /// @param nu The frequency in Hz
    /// @param restfreq The rest frequency in the same units
    /// @return The corresponding velocity in km/s

    double z = restfreq / nu - 1.;
    return redshiftToVel(z);
}

const double freqToHIVel(const double nu)
{
    /// @details Converts a frequency to the velocity of HI.
    /// @param nu The frequency in Hz
    /// @return The corresponding velocity in km/s

    return freqToVel(nu, nu0_HI);
}

const double velToFreq(const double vel, const double restfreq)
{
    /// @details Converts a velocity of a spectral line to a frequency.
    /// @param vel The velocity in km/s
    /// @param restfreq The rest frequency of the line
    /// @return The corresponding frequency in Hz

    double z = velToRedshift(vel);
    return redshiftToFreq(z, restfreq);
}

const double HIVelToFreq(const double vel)
{
    /// @details Converts a velocity of HI to a frequency.
    /// @param vel The velocity in km/s
    /// @return The corresponding frequency in Hz

    return velToFreq(vel, nu0_HI);
}

}

}
