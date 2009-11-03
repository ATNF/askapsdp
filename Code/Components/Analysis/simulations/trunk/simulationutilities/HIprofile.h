/// @file
///
/// Base class for spectral-line profile defintions
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
#ifndef ASKAP_SIMS_HI_PROFILE_H_
#define ASKAP_SIMS_HI_PROFILE_H_

#include <simulationutilities/Spectrum.h>
#include <iostream>

namespace askap {

    namespace simulations {

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
        /// @brief Convert a redshift to an observed HI frequency
        double redshiftToHIFreq(double z);
        /// @brief Convert an observed HI frequency to a recessional velocity
        double freqToHIVel(double nu);


        /// @brief A base class for spectral-line profiles
        /// @details This holds information about a spectral-line profile
        /// (usually HI). It stores the redshift and HI mass (a measure of the
        /// integrated flux), and provides methods for calculating the integrated
        /// flux, flux at a particular frequency and flux integrated between two
        /// frequencies.
        class HIprofile : public Spectrum {
            public:
                /// @brief Default constructor
                HIprofile();
                /// @brief Destructor
                virtual ~HIprofile() {};
                /// @brief Copy constructor
                HIprofile(const HIprofile& h);
                /// @brief Assignment operator
                HIprofile& operator= (const HIprofile& h);

                /// @brief Convert the HI mass to an integrated flux.
                double integratedFlux(double z, double mhi);

                /// @brief Return the redshift
                double redshift() {return itsRedshift;};
                /// @brief Return the HI mass
                double mHI() {return itsMHI;};

                /// @brief Return the flux at a given frequency - not used for the base class
                virtual double flux(double nu)  {return -177.;};
                /// @brief Return the flux integrated between two frequencies - not used for the base class
                virtual double flux(double nu1, double nu2)  {return -179.;};

                /// @brief Output the parameters for the source
                friend std::ostream& operator<< (std::ostream& theStream, HIprofile &prof);

            protected:
                /// @brief The redshift of the source
                double itsRedshift;
                /// @brief The HI mass of the source
                double itsMHI;

        };

    }

}

#endif

