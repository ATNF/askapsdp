/// @file FrtMetadataSource.cc
///
/// @brief This class is an adaptation of the original Ben's MetadataSource
/// but it deals with a different datatype. Perhaps we could've refactor this
/// code (e.g. with templates) to get a generic version. This class is intended
/// to receive fringe rotator and DRx specific messages to allow ingest pipeline
/// to control fringe rotation. We probably don't need buffering, but reuse
/// of Ben's thread-safe circular buffer seems to make things easier.
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
/// @author Max Voronkov <Max.Voronkov@csiro.au>

// Include own header file first
#include "FrtMetadataSource.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/shared_ptr.hpp"

ASKAP_LOGGER(logger, ".FrtMetadataSource");

using namespace askap;
using namespace askap::cp;
using namespace askap::cp::ingest;

FrtMetadataSource::FrtMetadataSource(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& topicManager,
        const std::string& topic,
        const std::string& adapterName,
        const unsigned int bufSize) :
    FrtMetadataReceiver(locatorHost, locatorPort, topicManager, topic, adapterName),
    itsBuffer(bufSize)
{
}

void FrtMetadataSource::receive(const std::map<std::string, int>& msg)
{
    ASKAPLOG_DEBUG_STR(logger, "Received fringe rotator metadata with " << msg.size()
            << " fields");

    // Make a copy of the message on the heap
    boost::shared_ptr<std::map<std::string, int> > frtMetadata(new std::map<std::string, int>(msg));

    // Add a pointer to the message to the back of the circular buffer.
    // Waiters are notified.
    itsBuffer.add(frtMetadata);
}

// Blocking
boost::shared_ptr<std::map<std::string, int> > FrtMetadataSource::next(const long timeout)
{
    return itsBuffer.next(timeout);
}

