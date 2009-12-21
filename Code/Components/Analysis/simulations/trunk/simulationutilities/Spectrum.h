/// @file
///
/// Base class for spectral profiles.
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
#ifndef ASKAP_SIMS_SPEC_H_
#define ASKAP_SIMS_SPEC_H_

#include <iostream>
#include <sstream>

#include <sourcefitting/Component.h>

namespace askap {

    namespace simulations {

        /// @brief Base class to hold information on a spectral profile.
        /// @details This class holds information on a profile that
        /// changes with spectral coordinate. This is the base class, that holds
        /// the sky position (RA & Dec), as well as shape information and a flux
        /// normalisation (the latter in an askap::sourcefitting::SubComponent object).
        class Spectrum {
            public:
                /// @brief Default constructor
                Spectrum() {};
                /// @brief Constructor using a line of input from an ascii file
                Spectrum(std::string &line);
                /// @brief Destructor
                virtual ~Spectrum() {};
                /// @brief Copy constructor
                Spectrum(const Spectrum& s);

		/// @brief Define using a line of input from an ascii file
		void define(std::string &line);

                /// @brief Return the right ascension
                std::string ra() {return itsRA;};
                /// @brief Return the decliination
                std::string dec() {return itsDec;};
                /// @brief Return the flux normalisation
                double fluxZero() {return itsComponent.peak();};
                /// @brief Return the major axis
                double maj() {return itsComponent.maj();};
                /// @brief Return the minor axis
                double min() {return itsComponent.min();};
                /// @brief Return the position angle
                double pa() {return itsComponent.pa();};

                /// @brief Set the flux normalisation
                void setFluxZero(float f) {itsComponent.setPeak(f);};
                /// @brief Set the major axis
                void setMaj(float f) {itsComponent.setMajor(f);};
                /// @brief Set the minor axis
                void setMin(float f) {itsComponent.setMinor(f);};
                /// @brief Set the position angle
                void setPA(float f) {itsComponent.setPA(f);};

                /// @brief Return the flux at a given frequency - not used for the base class
                virtual double flux(double freq)  {return -77.;};
                /// @brief Return the flux integrated between two frequencies - not used for the base class
                virtual double flux(double freq1, double freq2)  {return -79.;};

            protected:
                /// @brief The right ascension of the object
                std::string itsRA;
                /// @brief The declination of the object
                std::string itsDec;
                /// @brief The shape and flux information for the object.
                analysis::sourcefitting::SubComponent itsComponent;

        };

    }

}

#endif
