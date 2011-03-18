/// @file ConnectionWrapper.h
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNELCONNECTION_H
#define ASKAP_CP_CHANNELS_UVCHANNELCONNECTION_H

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/ExceptionListener.h"

// Local package includes
#include "mq/LibraryWrapper.h"

namespace askap {
namespace cp {
namespace channels {

    /// @brief Wraps the ActiveMQ connection and session for a single broker.
    /// @ingroup uvchannel
    class ConnectionWrapper : protected cms::ExceptionListener {

        public:
            /// Constructor
            /// @param[in] brokerURI    the URI used to identify and connect to
            /// the broker.
            ConnectionWrapper(const std::string& brokerURI);

            /// Destructor.
            ~ConnectionWrapper();

            /// @brief Get a pointer to the session object.
            ///
            /// @return a pointer to the session object. Note: The object is
            ///         still owned by the ConnectionWrapper and the pointer
            ///         is only valid while the ConnectionWrapper which
            ///         provided the pointer exists.
            cms::Session* getSession(void);

        protected:

            /// @brief This is an implementation concept. It is the method
            /// via which the client (i.e. this class) is notified of an
            /// exception condition with the CMS provider.
            /// @internal
            virtual void onException(const cms::CMSException &ex);

        private:

            // No support for assignment
            ConnectionWrapper& operator=(const ConnectionWrapper& rhs);

            // No support for copy constructor
            ConnectionWrapper(const ConnectionWrapper& src);

            // ActiveMQ library wrapper (manages the init/shutdown)
            LibraryWrapper mqlib;

            // ActiveMQ Connection
            boost::scoped_ptr<cms::Connection> itsConnection;

            // ActiveMQ Session
            boost::scoped_ptr<cms::Session> itsSession;
    };

};
};
};

#endif
