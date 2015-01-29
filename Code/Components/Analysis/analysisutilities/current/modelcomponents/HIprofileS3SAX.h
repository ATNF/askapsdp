/// @file
///
/// Contains class for describing HI profiles that come from the SKADS S3SAX database.
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
#ifndef ASKAP_SIMS_HI_PROFILE_S3SAX_H_
#define ASKAP_SIMS_HI_PROFILE_S3SAX_H_

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/HIprofile.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace askap {

namespace analysisutilities {

/// @brief The spectral profile of an HI emission line from the S3SAX database
/// @details This class holds all information required to
/// describe the spectral profile of an HI emission line for a source
/// extracted from the SKADS S3SAX database. The shape of the line is a
/// symmetric double-horn profile, completely specified by
/// information in the database. The functional specification is (from Obreschkow et al. 2009)
/// \f$ f(V) = k_3 \exp( -(|V|-k_1)^2/k_2 ), |V|\ge W_p/2\f$, giving the Gaussian tails, and
/// \f$ f(V) = k_5 / \sqrt{k_4-V^2}, |V| \le W_p/2\f$, given the dip between the peaks.
/// The constants are defined in terms of the five key parameters: \f$f_0, f_p, W_p, W_{50}, W_{20}\f$ by:
/// @li \f$ k_1 = \frac{1}{4}\frac{\ln(0.5)(W_p^2-W_{20}^2) + \ln(0.2)(W_{50}^2-W_p^2)}{ln(0.5)(W_p-W_{20}) + \ln(0.2)(W_{50}-W_p)}\f$
/// @li \f$ k_2 = \frac{1}{\ln(0.5)} (0.25(W_p^2-W_{50}^2) + k_1(W_{50}-W_p))\f$
/// @li \f$ k_3 = f_p \exp( (2k_1 - W_p)^2 / 4k_2 ) \f$
/// @li \f$ k_4 = \frac{W_p^2 f_p^2}{4(f_p^2 - f_0^2)} \f$
/// @li \f$ k_5 = f_0 \sqrt{k_4} \f$
///
/// @todo Put every single SKADS S3-SAX parameter into this class,
/// rather than just the relevant ones we get from mySQL

class HIprofileS3SAX : public HIprofile {
    public:
        /// @brief Default constructor
        HIprofileS3SAX() {};
        /// @brief Set up parameters using a line of input from an ascii file
        /// @details Constructs a HIprofileS3SAX object from a
        /// line of text from an ascii file. Uses the
        /// HIprofileS3SAX::define() function.
        HIprofileS3SAX(const std::string &line);
        /// @brief Destructor
        virtual ~HIprofileS3SAX() {};
        /// @brief Copy constructor
        HIprofileS3SAX(const HIprofileS3SAX& h);
        /// @brief Assignment operator
        HIprofileS3SAX& operator= (const HIprofileS3SAX& h);

        /// @brief Define using a line of input from an ascii file
        /// @details Defines a HIprofileS3SAX object from a line of
        /// text from an ascii file. This line should be formatted in
        /// the correct way to match the output from the appropriate
        /// python script. The columns should be: RA - DEC - Flux -
        /// Alpha - Beta - Major axis - Minor axis - Pos.Angle -
        /// redshift - HI Mass - f_0 - f_p - W_p - W_50 - W_20 (Alpha & Beta are the
        /// spectral index and spectral curvature - these are produced
        /// by the python scripts, but not used for the HI profiles,
        /// only Continuum profiles.)  The define() function is called
        /// to set up the profile description.
        /// @param line A line from the ascii input file
        void define(const std::string &line);

        /// @brief Set up the profile's parameters
        /// @details Sets up the \f$k_i\f$ parameters and the
        /// integrated fluxes, according to the equations described in
        /// the class definition. Need to have assigned the values of
        /// the other parameters (define() is usually called by the
        /// HIprofileS3SAX(std::string &line) constructor).
        void prepareForUse();

        /// @brief Return the integrated flux of the profile
        const double intFlux() {return itsIntFlux;};

        /// @brief Return the flux at a given frequency
        /// @details This function returns the flux value at a
        /// particular frequency, using the expressions shown in the
        /// comments for define(). This is a monochromatic flux, not
        /// integrated.
        /// @param nu The frequency, in Hz.
        /// @param istokes The stokes parameter. Anything other than 0 returns zero flux.
        /// @return The flux, in Jy.
        const double flux(const double nu, const int istokes = 0);

        /// @brief Return the flux integrated between two frequencies - not used for the base class
        /// @details This function returns the flux integrated between
        /// two frequencies. This can be used to calculate the flux in
        /// a given channel, for instance. The flux is divided by the
        /// frequency range, so that units of Jy are returned.
        /// @param nu1 One frequency, in Hz.
        /// @param nu2 The second frequency, in Hz.
        /// @param istokes The stokes parameter. Anything other than 0 returns zero flux.
        /// @return The flux, in Jy.
        const double fluxInt(const double nu1, const double nu2, const int istokes = 0);

        /// @brief Return the minimum & maximum frequencies affected by this source
        /// @details This function returns the minimum & maximum
        /// frequencies (in that order) that will be affected by a
        /// given source. This takes the limit of the exponential
        /// tails as the location where the flux drops below the
        /// minimum float value.
        /// @return A std::pair<double,double>, where the first value
        /// is the maximum velocity, and the second is the minimum
        /// velocity.
        const std::pair<double, double> freqLimits() const;

        /// @brief Output the parameters for the source
        void diagnostic(std::ostream& theStream) const;
        using HIprofile::print;
        void print(std::ostream& theStream) const;

        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, const HIprofileS3SAX &prof);

    private:
        /// @brief The flux of the two peaks, \f$f_p\f$
        double itsFluxPeak;
        /// @brief The flux at the central velocity, \f$f_0\f$
        double itsFlux0;
        /// @brief The velocity width of the two peaks, \f$W_p\f$
        double itsWidthPeak;
        /// @brief The velocity width at 50% of the peak flux, \f$W_{50}\f$
        double itsWidth50;
        /// @brief The velocity width at 20% of the peak flux, \f$W_{20}\f$
        double itsWidth20;
        /// @brief The integrated flux of the source in Jy km/s
        double itsIntFlux;
        /// @brief The integrated flux (in pseudo-units) of one of the profile tails
        double itsSideFlux;
        /// @brief The integrated flux (in pseudo-units) between the two peaks
        double itsMiddleFlux;
        /// @brief The \f$k_i\f$ parameters
        std::vector<double> itsKpar;

};

}

}

#endif
