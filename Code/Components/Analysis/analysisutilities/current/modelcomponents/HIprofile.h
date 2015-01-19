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

#include <modelcomponents/Spectrum.h>
#include <iostream>

namespace askap {

namespace analysisutilities {

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
        /// @details For an HI source of a given HI mass and a given
        /// redshift, this function calculates the integrated flux
        /// according to
        /// \f$S = 4.24\times10^{-6} M_{HI} / D^2\f$,
        /// where \f$D\f$ is the luminosity distance to that
        /// redshift.
        /// @param z The redshift
        /// @param mhi The HI mass in solar masses
        /// @return The integrated flux in Jy km/s
        double integratedFlux(double z, double mhi);

        /// @brief Return the redshift
        double redshift() {return itsRedshift;};
        /// @brief Return the HI mass
        double mHI() {return itsMHI;};

        bool freqRangeOK(double freq1, double freq2);

        /// @brief Return the flux at a given frequency - not used for the base class
        virtual double flux(double nu, int istokes = 0)
        {
            return -177.;
        };
        /// @brief Return the flux integrated between two frequencies
        /// - not used for the base class
        virtual double flux(double nu1, double nu2, int istokes = 0)
        {
            return -179.;
        };

        /// @brief Output the parameters for the source
        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, HIprofile &prof);

    protected:
        /// @brief The redshift of the source
        double itsRedshift;
        /// @brief The HI mass of the source
        double itsMHI;
        /// @brief The minimum frequency affected by the source
        double itsMinFreq;
        /// @brief The maximum frequency affected by the source
        double itsMaxFreq;


};

}

}

#endif

