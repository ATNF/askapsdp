/// @file EventChannelConnection.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTCHANNELCONNECTION_H
#define ASKAP_CP_EVENTCHANNEL_EVENTCHANNELCONNECTION_H

// System includes
#include <string>

// ASKAPsoft includes

namespace askap {
namespace cp {
namespace eventchannel {

class EventChannelConnection {
    public:
        EventChannelConnection& getSingletonInstance(void);
        EventChannelConnection& createSingletonInstance(const std::string& brokerURI);

        ~EventChannelConnection();

    private:

        EventChannelConnection(const std::string& brokerURI);

        // No support for assignment
        EventChannelConnection& operator=(const EventChannelConnection& rhs);

        // No support for copy constructor
        EventChannelConnection(const EventChannelConnection& src);
};

};
};
};

#endif
