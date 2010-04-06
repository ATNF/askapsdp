/// @file VisSource.h
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

#ifndef ASKAP_CP_VISSOURCE_H
#define ASKAP_CP_VISSOURCE_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/circular_buffer.hpp"
#include "boost/asio.hpp"
#include "cpcommon/VisPayload.h"

namespace askap {
    namespace cp {

        class VisSource
        {
            public:
                VisSource(const unsigned int port, const unsigned int bufSize);
                ~VisSource();

                // Blocking
                boost::shared_ptr<VisPayload> next(void);

            private:
                void start_receive(void);

                void handle_receive(const boost::system::error_code& error,
                        std::size_t bytes);

                void run(void);

                // Circular buffer of metadata payloads
                boost::circular_buffer< boost::shared_ptr<VisPayload> > itsBuffer;

                // Lock to protect itsBuffer
                boost::mutex itsMutex;

                // Condition variable signaling between threads
                boost::condition itsCondVar;

                // Service thread
                boost::shared_ptr<boost::thread> itsThread;

                // Used to request the service thread to stop
                bool itsStopRequested;

                // Boost io_service
                boost::asio::io_service itsIOService;

                // UDP socket
                boost::scoped_ptr<boost::asio::ip::udp::socket> itsSocket;

                boost::asio::ip::udp::endpoint itsRemoteEndpoint;

                boost::shared_ptr<VisPayload> itsRecvBuffer;
        };

    };
};

#endif
