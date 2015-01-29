/// @file
///
/// Contains class for describing HI profiles that come from the SKADS S3SEX database.
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
#ifndef ASKAP_SIMS_HI_PROFILE_S3SEX_H_
#define ASKAP_SIMS_HI_PROFILE_S3SEX_H_

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/HIprofile.h>
#include <iostream>
#include <math.h>

namespace askap {

namespace analysisutilities {

/// @brief Enumeration describing different parameters
enum SHAPEPARS {EDGE_SIG_MEAN, EDGE_SIG_SD, EDGE_SIG_MIN, EDGE_SIG_MAX,
                DIP_MIN, DIP_MAX, DIP_SIG_SCALE
               };
/// @brief The default values of the shape parameters
const double doubleHornShape[7] = {12.0, 6.0, 5., 20., 0.0, 0.3, 0.3};
/// @brief Enumeration describing types of galaxies in the S3SEX database
enum GALTYPE {RQAGN, FRI, FRII, SBG, SFG, GPS, UNKNOWN};
/// @brief Minimum rotational velocity for different galaxy types
const double vrotMin[5] = {0., 0., 0., 20., 40.};
/// @brief Maximum rotational velocity for different galaxy types
const double vrotMax[5] = {0., 0., 0., 70., 140.};

// sqrt(2pi), using, from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi),
const double rootTwoPi = 4. * M_SQRT1_2 / M_2_SQRTPI;

/// @brief The spectral profile of an HI emission line from the S3SEX database
/// @details This class holds all information required to
/// describe the spectral profile of an HI emission line for a source
/// extracted from the SKADS S3SEX database. The shape of the line is a
/// symmetric double-horn profile, made up of Gaussian-shaped
/// slopes, randomly generated when the define() function is called.
class HIprofileS3SEX : public HIprofile {
    public:
        /// @brief Default constructor
        HIprofileS3SEX() {};
        /// @brief Set up parameters using a line of input from an ascii file
        /// @details Constructs a HIprofileS3SEX object from a
        /// line of text from an ascii file. Uses the
        /// HIprofileS3SEX::define() function.
        HIprofileS3SEX(const std::string &line);
        /// @brief Set up parameters using the setup() function
        HIprofileS3SEX(const GALTYPE type,
                       const double z,
                       const double mhi,
                       const double maj,
                       const double min);
        HIprofileS3SEX(const GALTYPE type,
                       const double z,
                       const double mhi,
                       const double maj,
                       const double min,
                       const long componentNum,
                       const long galaxyNum);
        /// @brief Destructor
        virtual ~HIprofileS3SEX() {};
        /// @brief Copy constructor
        HIprofileS3SEX(const HIprofileS3SEX& h);
        /// @brief Assignment operator
        HIprofileS3SEX& operator= (const HIprofileS3SEX& h);

        /// @brief Define using a line of input from an ascii file
        /// @details Defines a HIprofileS3SEX object from a line of
        /// text from an ascii file. This line should be formatted in
        /// the correct way to match the output from the appropriate
        /// python script. The columns should be: RA - DEC - Flux -
        /// Alpha - Beta - Major axis - Minor axis - Pos.Angle -
        /// redshift - HI Mass - galaxy type. (Alpha & Beta are the
        /// spectral index and spectral curvature - these are produced
        /// by the python scripts, but not used for the HI profiles,
        /// only Continuum profiles.)  The define() function is called
        /// to set up the profile description.
        /// @param line A line from the ascii input file
        void define(const std::string &line);

        /// @brief Set up the profile's parameters
        /* void setup(GALTYPE type, double z, double mhi, double maj, double min); */
        void init();
        void setup();
        void prepareForUse();
        void prepareForUse(const long num1, const long num2);

        /// @brief What source type is this?
        const GALTYPE galtype() {return itsSourceType;};

        /// @brief Return the flux at a given frequency
        /// @details This function returns the flux value at a
        /// particular frequency, using the expressions shown in the
        /// comments for define(). This is a monochromatic flux, not
        /// integrated.
        /// @param nu The frequency, in Hz.
        /// @param istokes The stokes parameter. Anything other than 0 returns zero flux.
        /// @return The flux, in Jy.
        const double flux(const double nu, const int istokes = 0);

        /// @brief Return the flux integrated between two frequencies
        /// @details This function returns the flux integrated between
        /// two frequencies. This can be used to calculate the flux in
        /// a given channel, for instance. The flux is divided by the
        /// frequency range, so that units of Jy are returned.
        /// @param nu1 One frequency, in Hz.
        /// @param nu2 The second frequency, in Hz.
        /// @param istokes The stokes parameter. Anything other than 0 returns zero flux.
        /// @return The flux, in Jy.
        const double fluxInt(const double nu1, const double nu2, const int istokes = 0);

        /// @brief Output the parameters for the source
        void diagnostic(std::ostream& theStream);
        using HIprofile::print;
        void print(std::ostream& theStream) const;

        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, const HIprofileS3SEX &prof);

    private:
        void prepareForUse(double n1, double n2);

        /// @brief The spectral index
        double itsAlpha;
        /// @brief The spectral curvature
        double itsBeta;
        /// @brief The type of source as it appears in the database
        GALTYPE itsSourceType;
        /// @brief The central velocity of the source
        double itsVelZero;
        /// @brief The rotational velocity
        double itsVRot;
        /// @breif The projected velocity width (taking into account source inclination)
        double itsDeltaVel;
        /// @brief The amplitude of the dip between the horns (between 0 & 1)
        double itsDipAmp;
        /// @brief The sigma parameter for the outer edges of the profile
        double itsSigmaEdge;
        /// @brief The sigma parameter for the dip between the horns
        double itsSigmaDip;
        /// @brief The maximum value of the profile, before normalisation to flux units
        double itsMaxVal;
        /// @brief The integrated flux of the source, in Jy km/s
        double itsIntFlux;

        /// @brief The integrated flux (in pseudo-units) of one edge outside the peak
        double itsEdgeFlux;
        /// @brief The integrated flux (in pseudo-units) between the two peaks
        double itsMiddleFlux;
        /// @brief The integrated flux (in pseudo-units) of the full profile.
        double itsProfileFlux;
};

}

}

#endif
