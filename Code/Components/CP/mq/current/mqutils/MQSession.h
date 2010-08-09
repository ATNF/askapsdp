/// @file MQSession.h
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

#ifndef ASKAP_CP_MQBROKER_H
#define ASKAP_CP_MQBROKER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "cms/Connection.h"
#include "cms/Session.h"

namespace askap {
namespace cp {

class MQSession {
    public:
        MQSession(const std::string& brokerURI);

        ~MQSession();

        cms::Session& get(void);

    private:

        // No support for assignment
        MQSession& operator=(const MQSession& rhs);

        // No support for copy constructor
        MQSession(const MQSession& src);

        // ActiveMQ Connection
        boost::scoped_ptr<cms::Connection> itsConnection;

        // ActiveMQ Session
        boost::scoped_ptr<cms::Session> itsSession;
};

};
};

#endif
