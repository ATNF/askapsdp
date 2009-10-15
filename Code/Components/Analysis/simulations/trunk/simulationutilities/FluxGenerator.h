/// @file
///
/// Provides utility functions for simulations package
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

#include <simulationutilities/Spectrum.h>

#include <wcslib/wcs.h>

#include <vector>

namespace askap {

    namespace simulations {

        class FluxGenerator {
            public:
                FluxGenerator();
		FluxGenerator(int numChan);
                virtual ~FluxGenerator() {};
		/// @brief Copy constructor for FluxGenerator.
		FluxGenerator(const FluxGenerator& f);
		
		/// @brief Assignment operator for FluxGenerator.
		FluxGenerator& operator= (const FluxGenerator& f);

		void setNumChan(int num);
		int  nChan(){return itsNChan;};

		void addSpectrum(Spectrum &spec, double &x, double &y, wcsprm *wcs);
		void addSpectrumInt(Spectrum &spec, double &x, double &y, struct wcsprm *wcs);
		
		float getFlux(int i){return itsFluxValues.at(i);};

            protected:
		int   itsNChan;
		std::vector<float> itsFluxValues;

        };

    }

}

#endif
