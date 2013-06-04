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
#ifndef ASKAP_SIMS_SIMUTIL_H_
#define ASKAP_SIMS_SIMUTIL_H_

#include <simulationutilities/FluxGenerator.h>
#include <modelcomponents/Disc.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <duchamp/Utils/Section.hh>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <Common/ParameterSet.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

    namespace simulations {

        /// @brief Convert between a Gaussian's sigma and its FWHM
      inline double FWHMtoSIGMA(double f){return f / (2. * M_SQRT2 * sqrt(M_LN2));};
      inline double SIGMAtoFWHM(double s){return s * (2. * M_SQRT2 * sqrt(M_LN2));};

        /* /// @brief Return a normal random variable */
        /* float normalRandomVariable(float mean, float rms); */

	/// @brief Create a wcsprm struct from a parset
	struct wcsprm *parsetToWCS(const LOFAR::ParameterSet& theParset, const std::vector<unsigned int> &theAxes, const float &theEquinox, const float &theRestFreq, duchamp::Section &theSection);

        /// @brief Add a 2D Gaussian component to an array of fluxes.
        bool addGaussian(float *array, std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss, FluxGenerator &fluxG, bool integrate, bool verbose);

        /// @brief Add a 1D Gaussian (in the case of a thin 2D component) to an array of fluxes
        void add1DGaussian(float *array, std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss, FluxGenerator &fluxGen, bool verbose);

	bool addDisc(float *array, std::vector<unsigned int> axes, Disc &disc, FluxGenerator &fluxGen, bool verbose);

        /// @brief Add a single point source to an array of fluxes.
        bool addPointSource(float *array, std::vector<unsigned int> axes, double *pix, FluxGenerator &fluxGen, bool verbose);

        bool doAddGaussian(std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss);
        bool doAddPointSource(std::vector<unsigned int> axes, double *pix);
	bool doAddDisc(std::vector<unsigned int> axes, Disc &disc);

    }

}


#endif
