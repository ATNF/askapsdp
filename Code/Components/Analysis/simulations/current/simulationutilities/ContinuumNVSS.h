/// @file
///
/// Continuum source from the NVSS catalogue, using the full content as obtained from CDS, with ascii text/plain option
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
#ifndef ASKAP_SIMS_CONTNVSS_H_
#define ASKAP_SIMS_CONTNVSS_H_

#include <simulationutilities/Continuum.h>

namespace askap {

    namespace simulations {

        /// @brief A class to hold information for a continuum source taken from the NVSS.
        /// @details This class is a specialisation of
        ///simulations::Continuum, adapted for sources from the NRAO VLA Sky
        ///Survey (NVSS, Condon et al 1998).
        ///
        /// The flux at a given frequency is given by the relation:
        /// \f$F(\nu) = F(\nu_0) \left(\frac{\nu}{\nu_0}\right)^{\alpha + \beta\log(\nu/\nu_0)} \f$
        class ContinuumNVSS : public Continuum {
            public:
                /// @brief Default constructor
                ContinuumNVSS();
                /// @brief Constructor from Spectrum object
                ContinuumNVSS(Spectrum &s);
                /// @brief Constructor from Continuum object
                ContinuumNVSS(Continuum &c);
                /// @brief Set up parameters using a line of input from an ascii file
                ContinuumNVSS(std::string &line);
                /// @brief Define parameters directly
                ContinuumNVSS(float alpha, float beta, float nuZero) {defineSource(alpha, beta, nuZero);};
                /// @brief Define parameters directly
                ContinuumNVSS(float alpha, float beta, float nuZero, float fluxZero) {defineSource(alpha, beta, nuZero); setFluxZero(fluxZero);};
                /// @brief Destructor
                virtual ~ContinuumNVSS() {};
                /// @brief Copy constructor for ContinuumNVSS.
                ContinuumNVSS(const ContinuumNVSS& f);

                /// @brief Assignment operator for Continuum.
                ContinuumNVSS& operator= (const ContinuumNVSS& c);
                /// @brief Assignment operator for Continuum, using a Continuum object
                ContinuumNVSS& operator= (const Continuum& c);
                /// @brief Assignment operator for Continuum, using a Spectrum object
                ContinuumNVSS& operator= (const Spectrum& c);

		/// @brief Define using a line of input from an ascii file
		void define(std::string &line);

		void print(std::ostream& theStream);
                /// @brief Output the parameters for the source
                friend std::ostream& operator<< (std::ostream& theStream, ContinuumNVSS &cont);

		void printDetails(std::ostream& theStream);

            protected:

		float itsRadius;
		float itsXoff;
		float itsYoff;
		long  itsRecno;
		std::string itsField;
		float itsFieldXpos;
		float itsFieldYpos;
		std::string itsName;
		std::string itsRAstring;
		std::string itsDecstring;
		float itsRA_err;
		float itsDec_err;
		float itsS1400;
		float itsS1400_err;
		char itsMajorAxisLimit;
		float itsMajorAxis;
		char itsMinorAxisLimit;
		float itsMinorAxis;
		float itsPA;
		float itsMajorAxis_err;
		float itsMinorAxis_err;
		float itsPA_err;
		std::string itsFlagResidual;
		int itsResidualFlux;
		float itsPolFlux;
		float itsPolPA;
		float itsPolFlux_err;
		float itsPolPA_err;
		std::string itsInputLine;
      };

    }

}

#endif
