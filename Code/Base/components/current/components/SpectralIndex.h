/// @file SpectralIndex.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_COMPONENTS_SPECTRALINDEX_H
#define ASKAP_COMPONENTS_SPECTRALINDEX_H

// ASKAPsoft includes
#include "measures/Measures/MFrequency.h"

// Local package includes
#include "SpectralModel.h"
#include "ComponentType.h"

namespace askap {
namespace components {

/// A flux model that models the spectral variation (as the
/// frequency varies) with a spectral index.
///
/// Thread Safety:
/// While this class is immutable, it encapsulates an instance of casa::MFrequency
/// that is not known to be thread safe.
class SpectralIndex : public SpectralModel {
    public:

        /// Constructor
        ///
        /// @param[in] refFreq  the reference frequency
        /// @param[in] index    the spectral index (i.e. exponent) value
        ///
        /// @throws AskapError  if the "refFreq" parameter is zero or
        ///                     negative
        SpectralIndex(const casa::MFrequency& refFreq, double index);

        /// Returns the type of the component
        virtual ComponentType::SpectralShape type(void) const;

        /// Returns Return the scaling factor that indicates what proportion of
        /// the flux is at the specified frequency
        ///
        /// @param[in] centerFrequency  the frequency at which the flux scaling
        ///                             value is requested.
        ///
        /// @throws AskapError  if the "centerFrequency" parameter has a different
        ///                     reference frame to the reference frequency (as is
        ///                     returned by getRef())
        /// @throws AskapError  if the "centerFrequency" parameter is zero or
        ///                     negative
        virtual double sample(const casa::MFrequency& centerFrequency) const;

        /// Returns the reference frequency
        virtual const casa::MFrequency& getRefFreq(void) const;

        /// Returns the spectral index (i.e. exponent) value
        virtual double getIndex(void) const;

    private:
        const casa::MFrequency itsReferenceFreq;
        const double itsSpectralIndex;
};

}
}

#endif
