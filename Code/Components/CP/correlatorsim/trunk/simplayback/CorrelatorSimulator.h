/// @file CorrelatorSimulator.h
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

#ifndef ASKAP_CP_CORRELATORSIMULATOR_H
#define ASKAP_CP_CORRELATORSIMULATOR_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "ms/MeasurementSets/MeasurementSet.h"
#include "Common/ParameterSet.h"
#include "boost/scoped_ptr.hpp"

// Local package includes
#include "simplayback/ISimulator.h"
#include "simplayback/VisPort.h"

namespace askap
{
    namespace cp
    {
        class CorrelatorSimulator : public ISimulator
        {
            public:
                // Constructor
                CorrelatorSimulator(const LOFAR::ParameterSet& parset);

                // Destructor
                virtual ~CorrelatorSimulator();

                /// Fill the TimeTaggedTypedValueMap with metadata for the next integration
                /// cycle
                bool fillNext(void);

            private:
                // ParameterSet (configuration)
                const LOFAR::ParameterSet itsParset;

                // Cursor (index) for the main table of the measurement set
                unsigned int itsCurrentRow;

                // Measurement set
                boost::scoped_ptr<casa::MeasurementSet> itsMS;

                // Port for output of metadata
                boost::scoped_ptr<askap::cp::VisPort> itsPort;
        };
    };
};
#endif
