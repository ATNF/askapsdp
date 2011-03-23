/// @file UVChannelReceiver.h
/// @brief
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_RECIEVER_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_RECIEVER_H

// System includes
#include <string>
#include <deque>
#include <map>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"

// Local includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/UVChannelConsumer.h"
#include "uvchannel/IUVChannelListener.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief Higer level callback handler for VisChunk data and end-of-stream
/// signal. Also provides a queue of received VisCunk objects.
/// @ingroup uvdataaccess
class UVChannelReceiver : protected IUVChannelListener {

    public:
        /// Constructor
        UVChannelReceiver(const UVChannelConfig& channelConfig,
                          const std::string& channelName,
                          const casa::uInt startChan,
                          const casa::uInt nChan,
                          const casa::uInt maxQueueSize = 6);

        /// Destructor
        ~UVChannelReceiver();

        /// Checks whether there are more data available.
        ///
        /// @return False if the queue is empty and end-of-stream has been
        ///         signaled, otherwise True.
        casa::Bool hasMore(const casa::uInt chan) const;

        /// Get a pointer to the next VisChunk. This call blocks until a VisChunk is
        /// available, but will unblock and return a null pointer if end-of-stream is
        /// signaled by the producer.
        ///
        /// @return a pointer to the next VisChunk, or a null pointer if the queue is
        ///         empty and end-of-stream has been signaled.
        boost::shared_ptr<askap::cp::common::VisChunk> next(const casa::uInt chan);

    protected:

        /// @internal
        virtual void onMessage(const boost::shared_ptr<askap::cp::common::VisChunk> message,
                std::string destinationName);

        /// @internal
        virtual void onEndOfStream(std::string destinationName);

    private:

        // Channel Configuration
        const UVChannelConfig itsChannelConfig;

        // Channel Name
        const std::string itsChannelName;

        // Maximum size the queue will grow to before discarding incoming
        // messages.
        const casa::uInt itsMaxQueueSize;

        // False if end-of-stream has not been signaled, otherwise True.
        std::map<int, casa::Bool> itsEndOfStreamSignaled;

        // Consumer
        boost::scoped_ptr<UVChannelConsumer> itsConsumer;

        // Queue of incoming data. Data is pushed on to the back of the queue
        // by onMessage() and popped off the front of the queue by next().
        std::map<int, std::deque< boost::shared_ptr<askap::cp::common::VisChunk> > > itsQueue;

        // Mutex used for synchronising access to itsQueue and itsEndOfStreamSignaled
        mutable boost::mutex itsMutex;

        // Condition variable user for synchronising access to itsQueue
        boost::condition itsCondVar;

};

} // end of namespace channels
} // end of namespace cp
} // end of namespace askap

#endif
