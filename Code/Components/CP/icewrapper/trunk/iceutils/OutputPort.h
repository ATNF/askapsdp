/// @file OutputPort.h
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

#ifndef ASKAP_CP_OUTPUTPORT_H
#define ASKAP_CP_OUTPUTPORT_H

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <boost/shared_ptr.hpp>

// Local package includes
#include "iceutils/IPort.h"

namespace askap { namespace cp {

    template<class T, class P>
    class OutputPort : public IPort
    {
        public:

            /// @brief Constructor
            OutputPort(const Ice::CommunicatorPtr ic)
                : itsComm(ic)
            {
            }

            /// @brief Destructor.
            virtual ~OutputPort()
            {
            }

            virtual askap::cp::IPort::Direction getDirection(void) const
            {
                return IPort::OUT;
            };

            virtual void attach(const std::string& topic)
            {
                // Obtain the topic
                Ice::ObjectPrx obj = itsComm->stringToProxy("IceStorm/TopicManager");
                IceStorm::TopicManagerPrx topicManagerPrx =
                    IceStorm::TopicManagerPrx::checkedCast(obj);
                IceStorm::TopicPrx topicPrx;
                try {
                    topicPrx = topicManagerPrx->retrieve(topic);
                } catch (const IceStorm::NoSuchTopic&) {
                    topicPrx = topicManagerPrx->create(topic);
                } catch (const IceStorm::TopicExists&) {
                    // Something else has since created the topic
                    topicPrx = topicManagerPrx->retrieve(topic);
                }
                
                // Get the proxy
                Ice::ObjectPrx pub = topicPrx->getPublisher()->ice_oneway();
                itsProxy = P::uncheckedCast(pub);
            };

            virtual void detach(void)
            {
                itsProxy = 0;
            };

            virtual P getProxy(void)
            {
                if (itsProxy) {
                    return itsProxy;
                } else {
                    ASKAPTHROW(AskapError, "Proxy is not initialized");
                }
            };

            // Shared pointer definition
            typedef boost::shared_ptr<OutputPort> ShPtr;

        private:

            // Ice Communicator
            const Ice::CommunicatorPtr itsComm;

            // Proxy object via which publishing occurs
            P itsProxy;

            // Proxy to the topic manager
            IceStorm::TopicPrx itsTopicPrx;
    };

}; };

#endif
