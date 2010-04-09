/// @file InputPort.h
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

#ifndef ASKAP_CP_INPUTPORT_H
#define ASKAP_CP_INPUTPORT_H

// System includes
#include <string>
#include <deque>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/thread_time.hpp"

// Local package includes
#include "iceutils/IPort.h"

namespace askap { namespace cp {

    template<class T, class S>
    class InputPort : public IPort, public S
    {
        public:

            /// @brief Constructor
            InputPort(const Ice::CommunicatorPtr ic,
                    const Ice::ObjectAdapterPtr& adapter,
                    const unsigned int bufferSize)
                : itsComm(ic), itsAdapter(adapter), itsBufferSize(bufferSize)
            {
            }

            /// @brief Destructor.
            virtual ~InputPort()
            {
                detach();
            }

            /// @brief Returns the direction of this port, either input
            /// or output.
            ///
            /// @return Direction:IN or Direction:OUT.
            virtual askap::cp::IPort::Direction getDirection(void) const
            {
                return IPort::IN;
            };

            /// @brief Attach the port instance to a topic, where the topic
            /// is obtained from the specified topic manager.
            ///
            /// @param[in] topic the name of the topic to attach the port to.
            /// @param[in] topicManager the identity of the topic manager from
            ///                         where the topic subscription should be
            ///                         requested.
            virtual void attach(const std::string& topic, const std::string& topicManager) 
            {
                boost::mutex::scoped_lock lock(itsMutex);

                // Instantiate the object to register for callbacks
                const Ice::ObjectPtr callback = this;
                itsProxy = itsAdapter->addWithUUID(callback)->ice_oneway();

                // Obtain the topic
                Ice::ObjectPrx obj = itsComm->stringToProxy(topicManager);
                IceStorm::TopicManagerPrx topicManagerPrx =
                    IceStorm::TopicManagerPrx::checkedCast(obj);
                try {
                    itsTopicPrx = topicManagerPrx->retrieve(topic);
                } catch (const IceStorm::NoSuchTopic&) {
                    itsTopicPrx = topicManagerPrx->create(topic);
                } catch (const IceStorm::TopicExists&) {
                    // Something else has since created the topic
                    topicPrx = topicManagerPrx->retrieve(topic);
                }

                IceStorm::QoS qos;
                itsTopicPrx->subscribeAndGetPublisher(qos, itsProxy);
            };

            /// @brief Detach from the attached topic.
            /// This has no effect if a call to attach() has not yet been made,
            /// or if detach has already been called.
            virtual void detach(void) 
            {
                boost::mutex::scoped_lock lock(itsMutex);
                if (itsTopicPrx && itsProxy) {
                    itsTopicPrx->unsubscribe(itsProxy);
                }
            };

            // Timeout is in milliseconds
            virtual boost::shared_ptr<T> receive(unsigned long timeout = 0)
            {
                boost::mutex::scoped_lock lock(itsMutex);
                if (timeout == 0) {
                    while (itsBuffer.size() < 1) {
                        // While this call sleeps/blocks the mutex is released
                        itsCondVar.wait(lock); 
                    }
                } else {
                    boost::system_time const wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(timeout);

                    // Need to protect from spurious wakeups, so check both the predicate
                    // and the wakeup time, then wait again as required.
                    while ((itsBuffer.size() < 1) && (boost::get_system_time() < wakeupTime)) {
                        itsCondVar.timed_wait(lock, wakeupTime);
                    }
                    if (itsBuffer.size() < 1) {
                        return boost::shared_ptr<T>();
                    }
                }

                boost::shared_ptr<T> payload = itsBuffer.front();
                itsBuffer.pop_front();
                lock.unlock();

                // Notify all because the handler thread may be waiting for
                // space in the buffer to become available
                itsCondVar.notify_all();

                return payload;
            };

            virtual void handle(const T& payload, const Ice::Current& cur)
            {
                boost::mutex::scoped_lock lock(itsMutex);

                // Wait for space to become available in the buffer
                while (itsBuffer.size() >= BUFSZ) {
                    // While this call sleeps/blocks the mutex is released
                    itsCondVar.wait(lock); 
                }

                boost::shared_ptr<T> payloadCopy(new T(payload));

                itsBuffer.push_back(payloadCopy);
                lock.unlock();
                itsCondVar.notify_all();
            };

            /// Shared pointer definition
            typedef boost::shared_ptr<InputPort> ShPtr;

        private:

            // Ice Communicator
            const Ice::CommunicatorPtr itsComm;

            // An adapter
            Ice::ObjectAdapterPtr itsAdapter;

            // Buffer size (number of elements)
            const unsigned int itsBufferSize;

            // An Ice proxy to the object this class registers
            // (what happens to be itself)
            Ice::ObjectPrx itsProxy;

            // Proxy to the topic manager
            IceStorm::TopicPrx itsTopicPrx;

            // Buffer of received objects
            std::deque< boost::shared_ptr<T> > itsBuffer;

            // Lock to protect itsBuffer
            boost::mutex itsMutex;

            // Condition variable signaling between threads
            boost::condition itsCondVar;
    };

}; };

#endif
