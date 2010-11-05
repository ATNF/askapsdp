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

#include <simulationutilities/GaussianProfile.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <iostream>

namespace askap {

    namespace simulations {

        /// @brief A class for absorption-line profiles, aimed at FLASH simulations
        /// @details This holds information about an absorption-line profile
        /// that has a Gaussian shape. It uses the GaussianProfile class to do all 
        /// calculations, but assumes: the height of the Gaussian is peak optical 
        /// depth; the central location is in redshift; and the width is in velocity (km/s).
        class FLASHProfile : public GaussianProfile {
            public:
                /// @brief Default constructor
                FLASHProfile();
		/// @brief Specific constructor
		FLASHProfile(double &height, double &centre, double &width, AXISTYPE &type);
                /// @brief Destructor
                virtual ~FLASHProfile() {};
                /// @brief Copy constructor
                FLASHProfile(const FLASHProfile& h);
                /// @brief Assignment operator
                FLASHProfile& operator= (const FLASHProfile& h);

		void setFlagContinuumSubtract(bool f){itsFlagContinuumSubtracted = f;};
		bool flagContinuumSubtract(){return itsFlagContinuumSubtracted;};

		void define(std::string &line);

		void print(std::ostream& theStream);
                /// @brief Output the parameters for the source
                friend std::ostream& operator<< (std::ostream& theStream, FLASHProfile &prof);

            protected:
		bool itsFlagContinuumSubtracted;
		double itsContinuumFlux;
		double itsPeakOpticalDepth;
		double itsCentreRedshift;
		double itsVelocityWidth;

        };

    }

}

#endif

