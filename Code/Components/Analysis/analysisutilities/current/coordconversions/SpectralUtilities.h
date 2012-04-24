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

namespace askap {

    namespace analysisutilities {

        /// @brief The rest frequency of the fine-structure HI line in Hz
        const double nu0_HI = 1420405751.786;
        /// @brief The speed of light in km/s
        const double C_kms = 299792.458;
        /// @brief The hubble constant, in km/s/Mpc, from the WMAP results
        const double HUBBLE_WMAP = 71.;
        /// @brief The matter density from the WMAP results
        const double OMEGAM_WMAP = 0.27;
        /// @brief The dark energy density from the WMAP results
        const double OMEGAL_WMAP = 0.73;

        /// @brief Return the luminosity distance to redshift z for a given cosmology
        double luminosityDistance(double z, double H0 = HUBBLE_WMAP, double omegaM = OMEGAM_WMAP, double omegaL = OMEGAL_WMAP);
        /// @brief Convert a redshift to a distance for a given cosmology
        double redshiftToDist(double z, double H0 = HUBBLE_WMAP, double omegaM = OMEGAM_WMAP, double omegaL = OMEGAL_WMAP);
        /// @brief Convert a redshift to a line-of-sight velocity
        double redshiftToVel(double z);
	/// @brief Convert a line-of-sight velocity to a redshift
        double velToRedshift(double vel);
        /// @brief Convert a redshift to an observed frequency for a spectral-line
        double redshiftToFreq(double z, double restfreq);
        /// @brief Convert a redshift to an observed HI frequency
        double redshiftToHIFreq(double z);
	/// @brief Convert an observed frequency of a spectral-line to a redshift
	double freqToRedshift(double freq, double restfreq);
	/// @brief Convert an observed frequency of an HI spectral-line to a redshift
	double HIFreqToRedshift(double freq);
        /// @brief Convert an observed spectral-line frequency to a recessional velocity
        double freqToVel(double nu, double restfreq);
        /// @brief Convert an observed HI frequency to a recessional velocity
        double freqToHIVel(double nu);
        /// @brief Convert a recessional velocity to an observed frequency
        double velToFreq(double vel, double restfreq);
        /// @brief Convert a recessional velocity to an observed HI frequency
        double HIVelToFreq(double vel);

    }

}

#endif
