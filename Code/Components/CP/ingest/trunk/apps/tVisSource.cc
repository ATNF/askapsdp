/// @file tVisSource.cc
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
#include <unistd.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/shared_ptr.hpp"
#include "boost/asio.hpp"

// Local package includes
#include "ingestpipeline/sourcetask/VisSource.h"

// Using
using namespace askap::cp;
using boost::asio::ip::udp;

ASKAP_LOGGER(logger, ".tVisSource");

class VisOutPort {
    public:
        VisOutPort(const std::string& hostname,
                const std::string port) :
            itsSocket(itsIOService)
        {
            // Open the socket using UDP protocol
            boost::system::error_code operror;
            itsSocket.open(udp::v4(), operror);
            if (operror) {
                throw operror;
            }

            // Set an 8MB send buffer to help deal with the bursty nature of the
            // communication. The operating system may have some upper limit on
            // this number.
            boost::asio::socket_base::send_buffer_size option(1024 * 1024 * 8);
            boost::system::error_code soerror;
            itsSocket.set_option(option, soerror);
            if (soerror) {
                throw soerror;
            }

            // Query the nameservice
            udp::resolver resolver(itsIOService);
            udp::resolver::query query(udp::v4(), hostname, port);

            // Get the remote endpoint
            udp::endpoint destination = *resolver.resolve(query);

            // Connect - remembering this is a UDP socket, so connect does not
            // really connect. It just means the call to send doesn't need to 
            // specify the destination each time.
            boost::system::error_code coerror;
            itsSocket.connect(destination, coerror);
            if (coerror) {
                throw coerror;
            }
        }

        ~VisOutPort()
        {
            itsSocket.close();
        }

        void send(const VisPayload& payload)
        {
            boost::system::error_code error;
            itsSocket.send(boost::asio::buffer(&payload, sizeof(VisPayload)), 0, error);
            if (error) {
                std::cerr << "UDP send failed: " << error << std::endl;
            }
        }


    private:
        // io_service
        boost::asio::io_service itsIOService;

        // Network socket
        boost::asio::ip::udp::socket itsSocket;
};

int main(int argc, char *argv[])
{
    ASKAPLOG_INIT("tVisSource.log_cfg");

    const std::string hostname = "localhost";
    const unsigned int port = 3000;
    const std::string portStr = "3000";
    const unsigned int bufSize = 630 * 19 * 36 * 2; // Enough for two integrations

    std::cerr << "Creating instance of VisOutPort...";
    VisOutPort out(hostname, portStr);
    std::cerr << "Done" << std::endl;
    std::cerr << "Creating instance of VisSource (class under test)...";
    VisSource source(port, bufSize);
    sleep(1);
    std::cerr << "Done" << std::endl;

    // Test simple send, recv, send, recv case
    unsigned long time = 1234;;
    const int count = 10;
    for (int i = 0; i < count; ++i) {
        VisPayload outvis;
        outvis.timestamp = time;
        outvis.version = VISPAYLOAD_VERSION;
        std::cerr << "Publishing a VisPayload message...";
        out.send(outvis);

        std::cerr << "Done" << std::endl;

        std::cerr << "Waiting for class under test to receive it...";
        boost::shared_ptr<VisPayload> recvd = source.next();
        std::cerr << "Received" << std::endl;
        if (recvd->timestamp != time) {
            std::cerr << "Messages do not match" << std::endl;
            return 1;
        }
    }

    /*
    // Test the buffering abilities of MetadataSource
    time = 9876;
    for (int i = 0; i < bufSize; ++i) {
        askap::interfaces::TimeTaggedTypedValueMap metadata;
        metadata.timestamp = time;
        metadata.data["time"] = new askap::interfaces::TypedValueLong(askap::interfaces::TypeLong, time);
        std::cerr << "Publishing a metadata message...";
        out.send(metadata);
        std::cerr << "Done" << std::endl;
    }
    for (int i = 0; i < bufSize; ++i) {
        std::cerr << "Waiting for class under test to receive message...";
        boost::shared_ptr<askap::interfaces::TimeTaggedTypedValueMap> recvd = source.next();
        std::cerr << "Received" << std::endl;
        if (recvd->timestamp != time) {
            std::cerr << "Messages do not match" << std::endl;
            return 1;
        }
    }*/

    return 0;
}
