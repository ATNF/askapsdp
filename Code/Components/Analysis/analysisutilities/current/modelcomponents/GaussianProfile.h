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
#ifndef ASKAP_SIMS_GAUSS_PROFILE_H_
#define ASKAP_SIMS_GAUSS_PROFILE_H_

#include <modelcomponents/Spectrum.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <coordutils/SpectralUtilities.h>
#include <iostream>

namespace askap {

namespace analysisutilities {

/// @brief An enumeration describing what the x-axis of the Gaussian function is defined as.
enum AXISTYPE {PIXEL, FREQUENCY, VELOCITY, REDSHIFT};

const float defaultRestFreq = nu0_HI;
const AXISTYPE defaultAxisType = FREQUENCY;

/// @brief A base class for Gaussian spectral-line profiles
/// @details This holds information about a spectral-line profile
/// that has a Gaussian shape. It stores the velocity, FWHM, and peak intensity
/// integrated flux), and provides methods for calculating the integrated
/// flux, flux at a particular frequency and flux integrated between two
/// frequencies.
class GaussianProfile : public Spectrum {
    public:
        /// @brief Default constructor
        GaussianProfile();
        /// @brief Default constructor with rest freq
        GaussianProfile(const float restFreq);
        /// @brief Specific constructor
        GaussianProfile(const double &height,
                        const double &centre,
                        const double &width,
                        const AXISTYPE &type);
        /// @brief Constructor from line of input, with rest frequency
        GaussianProfile(const std::string &line, const float restfreq = defaultRestFreq);
        /// @brief Destructor
        virtual ~GaussianProfile() {};
        /// @brief Copy constructor
        GaussianProfile(const GaussianProfile& h);
        /// @brief Assignment operator
        GaussianProfile& operator= (const GaussianProfile& h);

        /// @details Defines a GaussianProfile object from a line of
        /// text from an ascii file. This line should be formatted in
        /// the correct way to match the output from the appropriate
        /// python script. The columns should be: RA - DEC - Flux -
        /// Peak height - central position - FWHM.
        /// @param line A line from the ascii input file
        virtual void define(const std::string &line);

        void setFreqLimits();
        virtual const bool freqRangeOK(const double freq1, const double freq2);

        void setAxisType(const AXISTYPE type) {itsAxisType = type;};
        void setRestFreq(const double freq) {itsRestFreq = freq;};

        /// @brief Return the flux at a given frequency
        virtual const double flux(const double nu, const int istokes = 0);
        /// @brief Return the flux integrated between two frequencies
        virtual const double fluxInt(const double nu1, const double nu2, const int istokes = 0);

        /// @brief Output the parameters for the source
        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, const GaussianProfile &prof);

    protected:
        casa::Gaussian1D<double> itsGaussian;
        AXISTYPE itsAxisType;
        double itsRestFreq;
        /// @brief The minimum frequency affected by the source
        double itsMinFreq;
        /// @brief The maximum frequency affected by the source
        double itsMaxFreq;
};

}

}

#endif

