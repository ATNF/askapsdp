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

// ASKAPsoft includes
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>

// Local package includes
#include "activities/IPort.h"

namespace askap { namespace cp {

    template<class T, class P>
    class InputPort : public IPort
    {
        public:

            /// @brief Constructor
            InputPort(const Ice::CommunicatorPtr ic)
                : itsComm(ic)
            {
            }

            /// @brief Destructor.
            virtual ~InputPort()
            {
            }

            virtual askap::cp::IPort::Direction getDirection(void)
            {
                return IPort::IN;
            };

            virtual void attach(const std::string& topic) 
            {
                // Obtain the topic
                Ice::ObjectPrx obj = itsComm->stringToProxy("IceStorm/TopicManager");
                IceStorm::TopicManagerPrx topicManager =
                    IceStorm::TopicManagerPrx::checkedCast(obj);
                IceStorm::TopicPrx topicPrx;
                try {
                    topicPrx = topicManager->retrieve(topic);
                } catch (const IceStorm::NoSuchTopic&) {
                    topicPrx = topicManager->create(topic);
                }

                // Get the proxy
                Ice::ObjectPrx pub = topicPrx->getPublisher()->ice_oneway();
                P interface = P::uncheckedCast(pub);
            };

            virtual void detach(const std::string& topic) 
            {
            };

        private:
            const Ice::CommunicatorPtr itsComm;
    };

}; };

#endif
