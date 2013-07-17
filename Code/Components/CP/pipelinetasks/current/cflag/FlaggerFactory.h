/// @file FlaggerFactory.h
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

#ifndef ASKAP_CP_PIPELINETASKS_FLAGGERFACTORY_H
#define ASKAP_CP_PIPELINETASKS_FLAGGERFACTORY_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "boost/shared_ptr.hpp"
#include "ms/MeasurementSets/MeasurementSet.h"

// Local package includes
#include "cflag/IFlagger.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief A factory that, given a Parameter Set, will create a flagging
/// flagger instance for each flagger enabled in the parset.
class FlaggerFactory {
    public:

        /// Appends one vector of flaggers to another. Vector "v2" is appended
        /// to vector "v1"
        ///
        /// @param[in,out]  v1  the vector that will be appended to.
        /// @param[in]      v2  the vector that will be appended to v1.
        static void appendFlaggers(vector< boost::shared_ptr<IFlagger> >& v1,
                                   const vector< boost::shared_ptr<IFlagger> > v2);

        /// Builds flagging flagger objects based on the configuration in
        /// the parameter set.
        ///
        /// @param[in] parset   the parameter set which contains an ASCII description
        ///                     of the flagging strategies to use.
        /// @param[in] ms       a reference to the measurement set that will
        ///                     be the subject of flagging.
        /// @return a vector containing pointers to the flagging strategies.
        static std::vector< boost::shared_ptr<IFlagger> > build(
            const LOFAR::ParameterSet& parset, const casa::MeasurementSet& ms);
};

}
}
}

#endif
