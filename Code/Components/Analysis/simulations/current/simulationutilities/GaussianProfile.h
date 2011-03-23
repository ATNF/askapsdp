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

#include <simulationutilities/Spectrum.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <iostream>

namespace askap {

    namespace simulations {

      /// @brief An enumeration describing what the x-axis of the Gaussian function is defined as.
      enum AXISTYPE {PIXEL, FREQUENCY, VELOCITY, REDSHIFT};

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
                GaussianProfile(float restFreq);
		/// @brief Specific constructor
		GaussianProfile(double &height, double &centre, double &width, AXISTYPE &type);
                /// @brief Destructor
                virtual ~GaussianProfile() {};
                /// @brief Copy constructor
                GaussianProfile(const GaussianProfile& h);
                /// @brief Assignment operator
                GaussianProfile& operator= (const GaussianProfile& h);

		virtual void define(std::string &line);

		void setAxisType(AXISTYPE type){itsAxisType=type;};
		void setRestFreq(double freq){itsRestFreq=freq;};

                /// @brief Return the flux at a given frequency
                virtual double flux(double nu);
                /// @brief Return the flux integrated between two frequencies
                virtual double flux(double nu1, double nu2);

                /// @brief Output the parameters for the source
                friend std::ostream& operator<< (std::ostream& theStream, GaussianProfile &prof);

            protected:
		casa::Gaussian1D<double> itsGaussian;
		AXISTYPE itsAxisType;
		double itsRestFreq;
        };

    }

}

#endif

