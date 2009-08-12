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
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>

// Local package includes
#include "activities/IPort.h"

namespace askap { namespace cp {

    template<class T, class S>
    class InputPort : public IPort, public S
    {
        public:

            /// @brief Constructor
            InputPort(const Ice::CommunicatorPtr ic, const Ice::ObjectAdapterPtr& adapter)
                : itsComm(ic), itsAdapter(adapter)
            {
            }

            /// @brief Destructor.
            virtual ~InputPort()
            {
                detach();
            }

            virtual askap::cp::IPort::Direction getDirection(void) const
            {
                return IPort::IN;
            };

            virtual void attach(const std::string& topic) 
            {
                boost::mutex::scoped_lock lock(itsMutex);

                // Instantiate the object to register for callbacks
                const Ice::ObjectPtr callback = this;
                itsProxy = itsAdapter->addWithUUID(callback)->ice_oneway();

                // Obtain the topic
                Ice::ObjectPrx obj = itsComm->stringToProxy("IceStorm/TopicManager");
                IceStorm::TopicManagerPrx topicManager =
                    IceStorm::TopicManagerPrx::checkedCast(obj);
                try {
                    itsTopicPrx = topicManager->retrieve(topic);
                } catch (const IceStorm::NoSuchTopic&) {
                    itsTopicPrx = topicManager->create(topic);
                }
                IceStorm::QoS qos;
                itsTopicPrx->subscribeAndGetPublisher(qos, itsProxy);
            };

            virtual void detach(void) 
            {
                boost::mutex::scoped_lock lock(itsMutex);
                if (itsTopicPrx && itsProxy) {
                    itsTopicPrx->unsubscribe(itsProxy);
                }
            };

            virtual T receive(void)
            {
                boost::mutex::scoped_lock lock(itsMutex);
                while (itsBuffer.size() < 1) {
                    // While this call sleeps/blocks the mutex is released
                    itsCondVar.wait(lock); 
                }

                T payload = itsBuffer.front();
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

                itsBuffer.push_back(payload);
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

            // An Ice proxy to the object this class registers
            // (what happens to be itself)
            Ice::ObjectPrx itsProxy;

            // Proxy to the topic manager
            IceStorm::TopicPrx itsTopicPrx;

            // Buffer of received objects
            std::deque<T> itsBuffer;

            // Lock to protect itsBuffer
            boost::mutex itsMutex;

            // Condition variable signaling between threads
            boost::condition itsCondVar;

            // Maximum number of objects to buffer in the port
            static const unsigned int BUFSZ = 2;
    };

}; };

#endif
