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

namespace askap {

    namespace simulations {

        /// @brief Base class to hold information on a spectral profile.
        /// @details This class holds information on a profile that
        /// changes with spectral coordinate. This is the base class, that holds
        /// the sky position (RA & Dec), as well as shape information and a flux
        /// normalisation.
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
		virtual void define(std::string &line);

                /// @brief Return the right ascension
                std::string ra() {return itsRA;};
                /// @brief Return the decliination
                std::string dec() {return itsDec;};
                /// @brief Return the flux normalisation
                double fluxZero() {return itsFlux;};
                /// @brief Return the major axis
                double maj() {return itsMaj;};
                /// @brief Return the minor axis
                double min() {return itsMin;};
                /// @brief Return the position angle
                double pa() {return itsPA;};

		virtual void setRA(double r, int prec=5);
		virtual void setRA(std::string r){itsRA=r;};
		virtual void setDec(double d, int prec=5);
		virtual void setDec(std::string d){itsDec=d;};
                /// @brief Set the flux normalisation
                void setFluxZero(float f) {itsFlux=f;};
                /// @brief Set the major axis
                void setMaj(float f) {itsMaj=f;};
                /// @brief Set the minor axis
                void setMin(float f) {itsMin=f;};
                /// @brief Set the position angle
                void setPA(float f) {itsPA=f;};
		/// @brief Make sure the major axis is the bigger
		void checkShape();

                /// @brief Return the flux at a given frequency - not used for the base class
                virtual double flux(double freq)  {return -77.;};
                /// @brief Return the flux integrated between two frequencies - not used for the base class
                virtual double flux(double freq1, double freq2)  {return -79.;};
		
		virtual void print(std::ostream& theStream, double ra, double dec, int prec=5);
		virtual void print(std::ostream& theStream, std::string ra, std::string dec);
		virtual void print(std::ostream& theStream);
		friend std::ostream& operator<< (std::ostream& theStream, Spectrum &spec);

            protected:
                /// @brief The right ascension of the object
                std::string itsRA;
                /// @brief The declination of the object
                std::string itsDec;
		float itsFlux;
		float itsMaj;
		float itsMin;
		float itsPA;

        };

    }

}

#endif
