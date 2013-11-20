/// @file tPubSub.cc
///
/// @copyright (c) 2013 CSIRO
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
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::ingest;
using namespace askap::cp::icewrapper;

ASKAP_LOGGER(logger, ".tPubSub");

class PubSubApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            const std::string locatorHost = config().getString("ice.locator_host");
            const std::string locatorPort = config().getString("ice.locator_port");
            const std::string topicManager = config().getString("icestorm.topicmanager");
            const std::string outtopic = config().getString("icestorm.outtopic");
            const std::string intopic = config().getString("icestorm.intopic");
            const std::string adapterName = "tPubSubAdapter";
            const int bufSize = 24;

            cout << "Creating a publisher on topic: " << outtopic << endl;
            FrtMetadataOutputPort out(locatorHost, locatorPort, topicManager, outtopic);
            cout << "Creating a subscriber on topic: " << intopic << endl;
            FrtMetadataSource source(locatorHost, locatorPort, topicManager, intopic,
                    adapterName, bufSize);

            // Send messages and check for responses
            while (true) {
                // Send
                map<std::string, int> amap = makeMap();
                cout << "Sending a map message" << endl;
                out.send(amap);

                // Check Subscriber Buffer
                 boost::shared_ptr<std::map<std::string, int> > recvd = source.next(0);
                 if (recvd.get()) {
                     cout << "Recieved a map with " << recvd->size() << " elements" << endl;
                 } else {
                     cout << "No data in receive buffer" << endl;
                 }

                 sleep(5);
            }

            return 0;
        }

        std::map<std::string, int> makeMap(void)
        {
            std::map<std::string, int> amap;
            amap["a"] = 1;
            amap["b"] = 2;
            return amap;
        }
};

int main(int argc, char *argv[])
{
  try {
    PubSubApp app;
    return app.main(argc, argv);
  }
  catch (const AskapError &) {
    return 1;
  }
}
