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

namespace analysisutilities {

/// @brief Type of component - point source, Gaussian, or disc
/// (uniform surface brightness out to an elliptical border)
enum ComponentType {POINT, GAUSSIAN, DISC};

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
        /// @details Constructs a Spectrum object from a line of
        /// text from an ascii file. Uses the Spectrum::define()
        /// function.
        Spectrum(std::string &line);
        /// @brief Destructor
        virtual ~Spectrum() {};
        /// @brief Copy constructor
        Spectrum(const Spectrum& s);

        /// @brief Define using a line of input from an ascii file
        /// @details Defines the Spectrum object from a line of
        /// text from an ascii file. The columns should accepted
        /// by this function are: RA - DEC - Flux - Major axis -
        /// Minor axis - Pos.Angle
        /// itsID is constructed from the RA & Dec
        /// @param line A line from the ascii input file
        virtual void define(const std::string &line);

        /// @brief Return the component type
        virtual ComponentType type() {if (itsMaj > 0.) return GAUSSIAN; else return POINT;};

        /// @brief Return the ID
        std::string id() {return itsID;};
        /// @brief Return the right ascension
        std::string ra() {return itsRA;};
        /// @brief Return the declination
        std::string dec() {return itsDec;};
        /// @brief Return the right ascension in degrees
        double raD();
        /// @brief Return the declination in degrees
        double decD();
        /// @brief Return the flux normalisation
        double fluxZero() {return itsFlux;};
        /// @brief Return the major axis
        double maj() {return itsMaj;};
        /// @brief Return the minor axis
        double min() {return itsMin;};
        /// @brief Return the position angle
        double pa() {return itsPA;};

        void setID(std::string s) {itsID = s;};

        ///@details This creates an ID string by combining the RA & Dec
        ///strings, separated by an underscore.
        void PosToID();

        virtual void setRA(double r, int prec = 5);
        virtual void setRA(std::string r) {itsRA = r;};
        virtual void setDec(double d, int prec = 5);
        virtual void setDec(std::string d) {itsDec = d;};
        /// @brief Set the flux normalisation
        void setFluxZero(float f) {itsFlux = f;};
        /// @brief Set the major axis
        void setMaj(float f) {itsMaj = f;};
        /// @brief Set the minor axis
        void setMin(float f) {itsMin = f;};
        /// @brief Set the position angle
        void setPA(float f) {itsPA = f;};
        /// @brief Make sure the major axis is the bigger
        void checkShape();

        /// @brief Calculate any parameters that are needed before making use of the class
        virtual void prepareForUse() {};

        virtual bool freqRangeOK(double freq1, double freq2) {return true;};

        /// @brief Return the flux at a given frequency - not used for the base class
        virtual double flux(double freq, int istokes = 0) = 0;
        /// @brief Return the flux integrated between two frequencies
        /// - not used for the base class
        virtual double fluxInt(double freq1, double freq2, int istokes = 0) = 0;

        virtual void print(std::ostream& theStream, double ra, double dec, int prec = 5);
        virtual void print(std::ostream& theStream, std::string ra, std::string dec);
        virtual void print(std::ostream& theStream);

        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, Spectrum &spec);

    protected:
        /// @brief A uniqe ID number or name
        std::string itsID;
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
