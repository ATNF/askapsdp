/// @file tFrtMetadataSource.cc
///
/// @details This is an adaptation of Ben's tMetadataSource to test communication 
/// classes related to fringe rotation metadata. The idea is very similar, but 
/// transferred data type is different (simple map of integers with the values
/// to be set for different registers).
///
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

// System includes
#include <iostream>
#include <string>
#include <map>

// ASKAPsoft includes
#include "askap/Application.h"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/shared_ptr.hpp"
#include "frtmetadata/FrtMetadataOutputPort.h"

// Local package includes
#include "ingestpipeline/phasetracktask/FrtMetadataSource.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::ingest;
using namespace askap::cp::icewrapper;

ASKAP_LOGGER(logger, ".tFrtMetadataSource");

class TestFrtMetaDataSourceApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            const std::string locatorHost = config().getString("ice.locator_host");
            const std::string locatorPort = config().getString("ice.locator_port");
            const std::string topicManager = config().getString("icestorm.topicmanager");
            const std::string topic = config().getString("icestorm.topic");
            const std::string adapterName = config().getString("ice.adapter_name");
            const int bufSize = 24;

            FrtMetadataOutputPort out(locatorHost, locatorPort, topicManager, topic);
            FrtMetadataSource source(locatorHost, locatorPort, topicManager, topic,
                    adapterName, bufSize);

            // Test simple send, recv, send, recv case
            const int count = 10;
            for (int i = 0; i < count; ++i) {
                std::map<std::string, int> frMsg;
                frMsg["count"] = i;
                frMsg["unity"] = 1;
                std::cout << "Publishing a fringe rotator message...";
                out.send(frMsg);
                std::cout << "Done" << std::endl;

                std::cout << "Waiting for class under test to receive it...";
                boost::shared_ptr<std::map<std::string, int> > recvd = source.next();
                std::cout << "Received" << std::endl;
                ASKAPASSERT(recvd);
                ASKAPCHECK(recvd->find("count")!=recvd->end(), "Field 'count' is not found in the received message");
                ASKAPCHECK(recvd->find("unity")!=recvd->end(), "Field 'unity' is not found in the received message");
                ASKAPCHECK(recvd->size() == 2, "Some garbage is present in the received message");
                ASKAPCHECK(recvd->operator[]("count") == i, "Value of the count field doesn't match for the message #"<< i + 1);
                ASKAPCHECK(recvd->operator[]("unity") == 1, "Value of the unity field doesn't match for the message #"<< i + 1);
            }

            // Test the buffering abilities of MetadataSource
            for (int i = 0; i < bufSize; ++i) {
                std::map<std::string, int> frMsg;
                frMsg["count"] = i;
                frMsg["unity"] = 1;
                std::cout << "Publishing a fringe rotator message...";
                out.send(frMsg);
                std::cout << "Done" << std::endl;
            }
            for (int i = 0; i < bufSize; ++i) {
                std::cout << "Waiting for class under test to receive message...";
                boost::shared_ptr<std::map<std::string, int> > recvd = source.next();
                std::cout << "Received" << std::endl;
                ASKAPASSERT(recvd);
                ASKAPCHECK(recvd->find("count")!=recvd->end(), "Field 'count' is not found in the received message");
                ASKAPCHECK(recvd->find("unity")!=recvd->end(), "Field 'unity' is not found in the received message");
                ASKAPCHECK(recvd->size() == 2, "Some garbage is present in the received message");
                ASKAPCHECK(recvd->operator[]("count") == i, "Value of the count field doesn't match for the message #"<< i + 1);
                ASKAPCHECK(recvd->operator[]("unity") == 1, "Value of the unity field doesn't match for the message #"<< i + 1);
            }

            return 0;
        }
};

int main(int argc, char *argv[])
{
  try {
    TestFrtMetaDataSourceApp app;
    return app.main(argc, argv);
  }
  catch (const AskapError &) {
    return 1;
  }
}
