/// @file SimPlayback.cc
///
/// @copyright (c) 2009 CSIRO
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

// Include own header file first
#include "SimPlayback.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisPayload.h"

// Local package includes
#include "cpinterfaces/TypedValues.h"
#include "simplayback/CorrelatorSimulator.h"
#include "simplayback/TosSimulator.h"
#include "simplayback/MetadataPort.h"
#include "simplayback/VisPort.h"

// Using
using namespace askap::cp;

ASKAP_LOGGER(logger, ".SimPlayback");

SimPlayback::SimPlayback(const LOFAR::ParameterSet& parset)
    : itsParset(parset), itsMetadataPort(parset), itsVisPort(parset)
{
}

SimPlayback::~SimPlayback()
{
}

void SimPlayback::run(void)
{
    // Get the filename for the measurement set and create a reader
    const std::string dataset = itsParset.getString("playback.dataset");
    ASKAPLOG_INFO_STR(logger, "Streaming dataset " << dataset);
    TosSimulator tosSim(dataset);
    CorrelatorSimulator corrSim(dataset);

    bool moreData = true;
    while (moreData) {
        // Structures to be populated and sent
        askap::interfaces::TimeTaggedTypedValueMap metadata;
        std::vector<askap::cp::VisPayload> visibilities;

        // Populate the structures
        moreData = tosSim.fillNext(metadata);
        bool corrMore = corrSim.fillNext(visibilities);
        if (corrMore != moreData) {
            ASKAPLOG_FATAL_STR(logger, "TosSimulator and CorrelatorSimulator state mismatch, aborting.");
            return;
        }

        // Send
        itsMetadataPort.send(metadata);
        itsVisPort.send(visibilities);
    }
    ASKAPLOG_INFO_STR(logger, "Completed streaming " << dataset);
}
