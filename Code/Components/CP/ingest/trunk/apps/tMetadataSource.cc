/// @file tMetadataSource.cc
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

// System includes
#include <iostream>
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "boost/shared_ptr.hpp"
#include "tosmetadata/MetadataOutputPort.h"

// CP Ice interfaces
#include "TypedValues.h"

// Local package includes
#include "ingestpipeline/sourcetask/MetadataSource.h"

// Using
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".tMetadataSource");

int main(int argc, char *argv[])
{
    ASKAPLOG_INIT("tMetadataSource.log_cfg");

    const std::string locatorHost = "localhost";
    const std::string locatorPort = "4061";
    const std::string topicManager = "IceStorm/TopicManager";
    const std::string topic = "tosmetadata";

    const std::string adapterName = argv[0];
    const int bufSize = 24;

    MetadataOutputPort out(locatorHost, locatorPort, topicManager, topic);
    MetadataSource source(locatorHost, locatorPort, topicManager, topic,
            adapterName, bufSize);

    // Test simple send, recv, send, recv case
    long time = 1234;
    const int count = 10;
    for (int i = 0; i < count; ++i) {
        askap::interfaces::TimeTaggedTypedValueMap metadata;
        metadata.timestamp = time;
        metadata.data["time"] = new askap::interfaces::TypedValueLong(askap::interfaces::TypeLong, time);
        std::cout << "Publishing a metadata message...";
        out.send(metadata);
        std::cout << "Done" << std::endl;

        std::cout << "Waiting for class under test to receive it...";
        boost::shared_ptr<askap::interfaces::TimeTaggedTypedValueMap> recvd = source.next();
        std::cout << "Received" << std::endl;
        if (recvd->timestamp != time) {
            std::cout << "Messages do not match" << std::endl;
            return 1;
        }
    }

    // Test the buffering abilities of MetadataSource
    time = 9876;
    for (int i = 0; i < bufSize; ++i) {
        askap::interfaces::TimeTaggedTypedValueMap metadata;
        metadata.timestamp = time;
        metadata.data["time"] = new askap::interfaces::TypedValueLong(askap::interfaces::TypeLong, time);
        std::cout << "Publishing a metadata message...";
        out.send(metadata);
        std::cout << "Done" << std::endl;
    }
    for (int i = 0; i < bufSize; ++i) {
        std::cout << "Waiting for class under test to receive message...";
        boost::shared_ptr<askap::interfaces::TimeTaggedTypedValueMap> recvd = source.next();
        std::cout << "Received" << std::endl;
        if (recvd->timestamp != time) {
            std::cout << "Messages do not match" << std::endl;
            return 1;
        }
    }

    return 0;
}
