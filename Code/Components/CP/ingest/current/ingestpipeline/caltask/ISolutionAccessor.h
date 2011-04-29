/// @file ISolutionAccessor.h
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

#ifndef ASKAP_CP_INGEST_ISOLUTIONACCESSOR_H
#define ASKAP_CP_INGEST_ISOLUTIONACCESSOR_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "casa/aipstype.h"
#include "casa/BasicSL/Complex.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief An interface for accessing calibration solutions.
class ISolutionAccessor {
    public:
        enum Pol {
            /// Polarisation 1
            XX,

            /// Polarisation 2
            YY
        };

        enum LeakageTerm {
            /// Leakage from feed 1 into feed 2
            D12,

            // Leakage from feed 2 into feed 1
            D21
        };

        /// Destructor.
        virtual ~ISolutionAccessor();

        /// Returns the leakage given the antenna number, beam number, and
        /// polarisation (X or Y), it returns the value of
        /// the parallel-hand gain (i.e. Gxx or Gyy)
        ///
        /// @param[in] ant  antenna id. This is the physical antenna ID, thus for ASKAP is
        ///                 in the range of 1-36.
        /// @param[in] beam beam id.
        /// @param[in] pol Either XX or YY
        /// @param[out] valid   used to indicate the validity of the returned data.
        ///                     Upon return the valid flag will be true if the
        ///                     returned data is good (usually indicating a valid
        ///                     gain exists, or false, usually in the case where
        ///                     a gain for the requested antenna/beam/pol does not
        ///                     exist.
        virtual casa::Complex getGain(casa::uInt ant,
                                      casa::uInt beam,
                                      ISolutionAccessor::Pol pol,
                                      casa::Bool& valid) const = 0;

        /// Returns the D-Terms (leakages) given the antenna number, beam number.
        /// Returns either d12 (Leakage from feed 1 into feed 2) or d21 (Leakage
        /// from feed 2 into feed 1) depending on the value of the "pol" parameter.
        ///
        /// @param[in] ant  antenna id. This is the physical antenna ID, thus for ASKAP is
        ///                 in the range of 1-36.
        /// @param[in] beam beam id
        /// @param[in] pol Either D12 or D21
        /// @param[out] valid   used to indicate the validity of the returned data.
        ///                     Upon return the valid flag will be true if the
        ///                     returned data is good (usually indicating a valid
        ///                     leakage exists, or false, usually in the case where
        ///                     a leakage for the requested antenna/beam/pol does not
        ///                     exist.
        virtual casa::Complex getLeakage(casa::uInt ant,
                                         casa::uInt beam,
                                         ISolutionAccessor::LeakageTerm term,
                                         casa::Bool& valid) const = 0;

        /// Returns the bandpass given the antenna number, beam number, channel
        /// number and polarisation.
        ///
        /// @param[in] ant  antenna id. This is the physical antenna ID, thus for ASKAP is
        ///                 in the range of 1-36.
        /// @param[in] beam beam id
        /// @param[in] channel channel number
        /// @param[in] pol Either XX or YY
        /// @param[out] valid   used to indicate the validity of the returned data.
        ///                     Upon return the valid flag will be true if the
        ///                     returned data is good (usually indicating a valid
        ///                     bandpass exists, or false, usually in the case where
        ///                     a bandpass for the requested antenna/beam/pol does not
        ///                     exist.
        virtual casa::Complex getBandpass(casa::uInt ant,
                                          casa::uInt beam,
                                          casa::uInt chan,
                                          ISolutionAccessor::Pol pol,
                                          casa::Bool& valid) const = 0;

        /// Shared pointer definition
        typedef boost::shared_ptr<ISolutionAccessor> ShPtr;

};

}
}
}

#endif
