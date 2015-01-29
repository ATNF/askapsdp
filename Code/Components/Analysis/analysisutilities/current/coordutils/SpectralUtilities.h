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

#ifndef ASKAP_SIMS_SPECTRALUTILS_H_
#define ASKAP_SIMS_SPECTRALUTILS_H_

#include <iostream>
#include <cosmology/Cosmology.h>

namespace askap {

namespace analysisutilities {

/// @brief The rest frequency of the fine-structure HI line in Hz
const double nu0_HI = 1420405751.786;

/// @brief Convert a redshift to a distance for a given cosmology
const double redshiftToDist(const double z,
                            const cosmology::Cosmology cosmology = cosmology::Cosmology());
/// @brief Convert a redshift to a line-of-sight velocity
const double redshiftToVel(const double z);
/// @brief Convert a line-of-sight velocity to a redshift
const double velToRedshift(const double vel);
/// @brief Convert a redshift to an observed frequency for a spectral-line
const double redshiftToFreq(const double z, const double restfreq);
/// @brief Convert a redshift to an observed HI frequency
const double redshiftToHIFreq(const double z);
/// @brief Convert an observed frequency of a spectral-line to a redshift
const double freqToRedshift(const double freq, const double restfreq);
/// @brief Convert an observed frequency of an HI spectral-line to a redshift
const double HIFreqToRedshift(const double freq);
/// @brief Convert an observed spectral-line frequency to a recessional velocity
const double freqToVel(const double nu, const double restfreq);
/// @brief Convert an observed HI frequency to a recessional velocity
const double freqToHIVel(const double nu);
/// @brief Convert a recessional velocity to an observed frequency
const double velToFreq(const double vel, const double restfreq);
/// @brief Convert a recessional velocity to an observed HI frequency
const double HIVelToFreq(const double vel);

}

}

#endif
