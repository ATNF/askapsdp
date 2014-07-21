/// @file TopicConfig.cc
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

// Include own header file first
#include "TopicConfig.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

// Local package includes

ASKAP_LOGGER(logger, ".TopicConfig");

using namespace askap;
using namespace askap::cp::ingest;

TopicConfig::TopicConfig(const std::string& registryHost,
                         const std::string& registryPort,
                         const std::string& topicManager,
                         const std::string& topic)
        : itsRegistryHost(registryHost), itsRegistryPort(registryPort),
        itsTopicManager(topicManager), itsTopic(topic)
{
}

std::string TopicConfig::registryHost(void) const
{
    return itsRegistryHost;
}

std::string TopicConfig::registryPort(void) const
{
    return itsRegistryPort;
}

std::string TopicConfig::topicManager(void) const
{
    return itsTopicManager;
}

std::string TopicConfig::topic(void) const
{
    return itsTopic;
}
