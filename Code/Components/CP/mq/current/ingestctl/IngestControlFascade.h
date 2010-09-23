/// @file IngestControlFascade.h
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

#ifndef ASKAP_CP_MQ_INGESTCONTROLFASCADE_H
#define ASKAP_CP_MQ_INGESTCONTROLFASCADE_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"    // LOFAR
#include "cms/Destination.h"
#include "cms/MessageProducer.h"
#include "cms/MapMessage.h"

// Local package includes
#include "mqutils/MQSession.h"

namespace askap {
namespace cp {
namespace mq {

class IngestControlFascade {
    public:
        /// @brief Constructor
        IngestControlFascade(const std::string& brokerURI,
                const std::string& destURI);

        /// @brief Destructor
        ~IngestControlFascade();

        // start
        void start(const LOFAR::ParameterSet& parset);

        // abort
        void abort(void);

        // State enumeration
        enum PipelineState {
            IDLE,
            STARTING,
            RUNNING,
            SHUTTING_DOWN
        };

        // getState
        PipelineState getState(void);

        // shutdown
        void shutdown(void);

    private:

        /// Generate UUIDs for message correlation ID
        /// @return a string containing a unique identifier
        std::string getUUID(void);

        /// Given a filename for a LOFAR style parameter set, and a pointer to a
        /// map message, this function reads each key/value pair from the parset
        /// and adds it to the map message.
        ///
        /// @param[in]  message the map message to which the key/value pairs will
        ///             be added
        /// @param[in]  parsetFile the filename for the parameter set
        void addParset(cms::MapMessage* message, const LOFAR::ParameterSet& parset);

        // timeout is in milliseconds
        cms::MapMessage* sendRequest(cms::Message* request,
                const int timeout = 10000);

        boost::scoped_ptr<MQSession> itsMQSession;

        boost::scoped_ptr<cms::Destination> itsDestination;

        boost::scoped_ptr<cms::MessageProducer> itsProducer;
};

};
};
};

#endif
