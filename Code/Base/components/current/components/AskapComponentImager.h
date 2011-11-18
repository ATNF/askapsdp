/// @file AskapComponentImager.h
///
/// @copyright (c) 2011 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_COMPONENTS_ASKAPCOMPONENTIMAGER_H
#define ASKAP_COMPONENTS_ASKAPCOMPONENTIMAGER_H

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "images/Images/ImageInterface.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/SkyComponent.h"
#include "images/Images/ImageInterface.h"
#include "coordinates/Coordinates/DirectionCoordinate.h"
#include "measures/Measures/Stokes.h"
#include "casa/Arrays/IPosition.h"
#include "components/ComponentModels/Flux.h"

namespace askap {
namespace components {

/// @brief Project the componentlist onto the image.
/// This class is designed to be interface compatible (in the method signature sense,
/// not in the OO sense, since there is no interface class) with the casacore
/// Component Imager. This class is based on the implementation of the casacore
/// ComponentImager however is implemented in a manner which should be
/// more performant.
class AskapComponentImager {
    public:
        /// Project the componentlist onto the image.
        ///
        /// @param[inout] image the image onto which the components will be projected.
        /// @param[in] list the list of components to project.
        template<class T>
        static void project(casa::ImageInterface<T>& image,
                            const casa::ComponentList& list,
                            const unsigned int term = 0);

    private:
        template<class T>
        static void projectPointShape(casa::ImageInterface<T>& image,
                                      const casa::SkyComponent& c,
                                      const casa::Int latAxis, const casa::Int longAxis,
                                      const casa::DirectionCoordinate& dirCoord,
                                      const casa::Int freqAxis, const casa::uInt freqIdx,
                                      const casa::Flux<casa::Double>& flux,
                                      const casa::Int polAxis, const casa::uInt polIdx,
                                      const casa::Stokes::StokesTypes& stokes);

        template<class T>
        static void projectGaussianShape(casa::ImageInterface<T>& image,
                                         const casa::SkyComponent& c,
                                         const casa::Int latAxis, const casa::Int longAxis,
                                         const casa::DirectionCoordinate& dirCoord,
                                         const casa::Int freqAxis, const casa::uInt freqIdx,
                                         const casa::Flux<casa::Double>& flux,
                                         const casa::Int polAxis, const casa::uInt polIdx,
                                         const casa::Stokes::StokesTypes& stokes);

        static casa::IPosition makePosition(const casa::Int latAxis, const casa::Int longAxis,
                                            const casa::Int spectralAxis, const casa::Int polAxis,
                                            const casa::uInt latIdx, const casa::uInt longIdx,
                                            const casa::uInt spectralIdx, const casa::uInt polIdx);

        static casa::Flux<casa::Double> makeFlux(const casa::SkyComponent& c,
                const casa::MFrequency& chanFrequency,
                const unsigned int term);
};

// Explicit instantiations exist for float and double types only
extern template void AskapComponentImager::project(casa::ImageInterface<float>&,
        const casa::ComponentList&, const unsigned int);
extern template void AskapComponentImager::project(casa::ImageInterface<double>&,
        const casa::ComponentList&, const unsigned int);
}
}

#endif
