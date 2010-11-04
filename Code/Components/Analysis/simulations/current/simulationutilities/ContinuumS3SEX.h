/// @file
///
/// Provides utility functions for simulations package
///
/// @copyright (c) 2007 CSIRO
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
#ifndef ASKAP_SIMS_CONT_S3SEX_H_
#define ASKAP_SIMS_CONT_S3SEX_H_

#include <simulationutilities/Spectrum.h>
#include <simulationutilities/Continuum.h>

namespace askap {

    namespace simulations {

        /// @brief A class to hold spectral information for a continuum spectrum.
        /// @details This class holds information on the continuum
        /// properties of a spectral profile. The information kept is the spectral
        /// index alpha, the spectral curvature parameter beta, and the
        /// normalisation frequency. It inherits the position, shape and flux
        /// normalisation from Spectrum.
        ///
        /// The flux at a given frequency is given by the relation:
        /// \f$F(\nu) = F(\nu_0) \left(\frac{\nu}{\nu_0}\right)^{\alpha + \beta\log(\nu/\nu_0)} \f$
        class ContinuumS3SEX : public Continuum {
            public:
                /// @brief Default constructor
                ContinuumS3SEX();
                /// @brief Constructor from Spectrum object
                ContinuumS3SEX(Spectrum &s);
                /// @brief Constructor from Continuum object
                ContinuumS3SEX(Continuum &c);
                /// @brief Set up parameters using a line of input from an ascii file
                ContinuumS3SEX(std::string &line);
                /// @brief Define parameters directly
                ContinuumS3SEX(float alpha, float beta, float nuZero) {defineSource(alpha, beta, nuZero);};
                /// @brief Define parameters directly
                ContinuumS3SEX(float alpha, float beta, float nuZero, float fluxZero) {defineSource(alpha, beta, nuZero); setFluxZero(fluxZero);};
                /// @brief Destructor
                virtual ~ContinuumS3SEX() {};
                /// @brief Copy constructor for ContinuumS3SEX.
                ContinuumS3SEX(const ContinuumS3SEX& f);

                /// @brief Assignment operator for Continuum.
                ContinuumS3SEX& operator= (const ContinuumS3SEX& c);
                /// @brief Assignment operator for Continuum, using a Continuum object
                ContinuumS3SEX& operator= (const Continuum& c);
                /// @brief Assignment operator for Continuum, using a Spectrum object
                ContinuumS3SEX& operator= (const Spectrum& c);

		/// @brief Define using a line of input from an ascii file
		void define(std::string &line);

                /// @brief Set up the profile's parameters
                void defineSource(float alpha, float beta, float nuZero);


            protected:
		long itsComponentNum;
		long itsGalaxyNum;
		short itsStructure;
		
		
        };

    }

}

#endif
