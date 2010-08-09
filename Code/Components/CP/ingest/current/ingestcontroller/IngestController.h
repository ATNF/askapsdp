/// @file IngestController.h
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

#ifndef ASKAP_CP_INGESTCONTROLLER_H
#define ASKAP_CP_INGESTCONTROLLER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/MessageListener.h"
#include "cms/ExceptionListener.h"
#include "cms/Message.h"
#include "cms/CMSException.h"

// Local package includes
#include "ingestpipeline/IngestPipeline.h"

namespace askap {
namespace cp {

class IngestController : public cms::MessageListener,
    public cms::ExceptionListener {

    public:
        /// @brief Constructor.
        IngestController(const std::string& brokerURI,
                const std::string& topicURI);

        /// @brief Destructor.
        ~IngestController();

        /// @brief Start running the controller.
        void run(void);

    private:

        // Handle the start command
        void startCmd(const cms::Message* message);

        // Handle the abort command
        void abortCmd(const cms::Message* message);
        
        // Handle the status request command
        void statusCmd(const cms::Message* message);

        // Called asynchronously when a new message is received, the
        // message reference can be to any of the Message types.
        virtual void onMessage(const cms::Message* message);

        // ActiveMQ exception handler. Called when an exception occurs.
        virtual void onException(const cms::CMSException& ex);

        // Send a response back to the sender
        void sendResponse(const cms::Message* request,
                const std::string& responseMsgType,
                const std::string& message);

        // Build a LOFAR ParameterSet given a MapMessage where all the entries
        // in the map are of type string.
        LOFAR::ParameterSet buildParset(const cms::MapMessage* message);

        // State enumeration
        enum PipelineState {
            IDLE,
            STARTING,
            RUNNING,
            SHUTTING_DOWN
        };

        // State
        PipelineState itsState;

        // Topic URI
        std::string itsTopicURI;

        // ActiveMQ Connection
        boost::scoped_ptr<cms::Connection> itsConnection;

        // ActiveMQ Session
        boost::scoped_ptr<cms::Session> itsSession;

        // IngestPipeline instance to control
        boost::scoped_ptr<IngestPipeline> itsPipeline;

        // Parameter set
        LOFAR::ParameterSet itsParset;

        // No support for assignment
        IngestController& operator=(const IngestController& rhs);

        // No support for copy constructor
        IngestController(const IngestController& src);

};

};
};

#endif
