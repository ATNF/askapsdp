/// @file TosSimulator.h
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_TOSSIMULATOR_H
#define ASKAP_CP_TOSSIMULATOR_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "ms/MeasurementSets/MeasurementSet.h"

// Local package includes
#include "cpinterfaces/TypedValues.h"

namespace askap
{
    namespace cp
    {
        class TosSimulator
        {
            public:
                /// Constructor
                TosSimulator(const std::string& filename);

                /// Destructor
                ~TosSimulator();

                /// Fill the TimeTaggedTypedValueMap with metadata for the next integration
                /// cycle
                bool fillNext(askap::interfaces::TimeTaggedTypedValueMap& metadata);

            private:
                // Utility function, used to build a string out of two 
                std::string makeMapKey(const std::string &prefix, const std::string &suffix);

                // Measurement set
                casa::MeasurementSet itsMS;

                // Cursor (index) for the main table of the measurement set
                unsigned int itsCurrentRow;
        };
    };
};
#endif
