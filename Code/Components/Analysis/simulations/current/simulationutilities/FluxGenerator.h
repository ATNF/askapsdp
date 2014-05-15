/// @file
///
/// Provides mechanism for calculating flux values of a set of spectral channels.
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
#ifndef ASKAP_SIMS_FLUXGEN_H_
#define ASKAP_SIMS_FLUXGEN_H_

#include <modelcomponents/Spectrum.h>
using namespace askap::analysisutilities;

#include <wcslib/wcs.h>

#include <vector>

namespace askap {

    namespace simulations {

        /// @brief A class to generate fluxes for a spectral profile at a given frequency.
        /// @brief This class holds the set of flux values over a range
        /// of channels for a given spectral profile (or set of profiles, as they
        /// can be added together). The aim of this class is to provide a way of
        /// storing the spectral profile of a source that can be used many times
        /// to assign fluxes to an extended source.
        class FluxGenerator {
            public:
                /// @brief Default constructor
                FluxGenerator();
                /// @brief Constructor based on a certain number of channels & Stokes parameters
                FluxGenerator(size_t numChan, size_t numStokes=1);
                /// @brief Destructor
                virtual ~FluxGenerator() {};
                /// @brief Copy constructor for FluxGenerator.
                FluxGenerator(const FluxGenerator& f);

                /// @brief Assignment operator for FluxGenerator.
                FluxGenerator& operator= (const FluxGenerator& f);

                /// @brief Set the number of channels
                void setNumChan(size_t num);
                /// @brief Set the number of Stokes parameters
                void setNumStokes(size_t num);
                /// @brief Return the number of channels
                size_t  nChan() {return itsNChan;};
                /// @brief Return the number of Stokes parameters
                size_t  nStokes() {return itsNStokes;};
		/// @brief Set the flux values to zero
		void zero();

                /// @brief Add a spectral profile to the flux values, using single flux points
                void addSpectrum(Spectrum *spec, double &x, double &y, wcsprm *wcs);
                /// @brief Add a spectral profile to the flux values, integrating over the channels
                void addSpectrumInt(Spectrum *spec, double &x, double &y, struct wcsprm *wcs);

                /// @brief Return the flux in channel i and Stokes plane s
                float getFlux(size_t i, size_t s=0) {return itsFluxValues.at(s).at(i);};

            protected:
                /// @brief Number of channels
                size_t itsNChan;
		size_t itsNStokes;
                /// @brief The set of flux values for each channel & Stokes parameter
		std::vector< std::vector<float> > itsFluxValues;

        };

    }

}

#endif
