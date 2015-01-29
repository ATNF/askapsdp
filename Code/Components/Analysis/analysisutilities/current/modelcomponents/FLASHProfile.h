/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#ifndef ASKAP_SIMS_FLASH_PROFILE_H_
#define ASKAP_SIMS_FLASH_PROFILE_H_

#include <modelcomponents/GaussianProfile.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <iostream>

namespace askap {

namespace analysisutilities {

/// @brief A class for absorption-line profiles, aimed at FLASH simulations
/// @details This holds information about an absorption-line profile
/// that has a Gaussian shape. It uses the GaussianProfile class to do all
/// calculations, but assumes: the height of the Gaussian is peak optical
/// depth; the central location is in redshift; and the width is in velocity (km/s).
class FLASHProfile : public GaussianProfile {
    public:
        /// @brief Default constructor
        FLASHProfile();
        /// @brief Default constructor with rest freq
        FLASHProfile(const float restFreq);
        /// @brief Specific constructor
        FLASHProfile(const double &height,
                     const double &centre,
                     const double &width,
                     const AXISTYPE &type);
        /// @brief Constructor from line of input, with rest frequency
        FLASHProfile(const std::string &line, const float restfreq = defaultRestFreq);
        /// @brief Destructor
        virtual ~FLASHProfile() {};
        /// @brief Copy constructor
        FLASHProfile(const FLASHProfile& h);
        /// @brief Assignment operator
        FLASHProfile& operator= (const FLASHProfile& h);

        /// @details Defines a FLASHProfile object from a line of
        /// text from an ascii file. This line should be formatted in
        /// the correct way to match the output from the appropriate
        /// python script. The columns should be: RA - DEC - Flux -
        /// Peak optical depth - central position - FWHM.
        /// The flux is used to scale the depth of the Gaussian, and,
        /// if itsFlagContinuumSubtracted is true, the component flux
        /// is then set to zero.
        /// The central position is assumed to be in units of redshift.
        /// The FWHM is assumed to be in units of velocity [km/s],
        /// and is converted to redshift.
        /// @param line A line from the ascii input file
        void define(const std::string &line);

        void prepareForUse();

        using Spectrum::print;
        void print(std::ostream& theStream) const;

        /// @brief Output the parameters for the source
        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, const FLASHProfile &prof);

    protected:
        bool   itsFlagContinuumSubtracted;
        long   itsComponentNum;
        double itsContinuumFlux;
        double itsPeakOpticalDepth;
        double itsCentreRedshift;
        double itsVelocityWidth;

};

}

}

#endif

