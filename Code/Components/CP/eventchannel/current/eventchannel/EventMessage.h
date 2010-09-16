/// @file EventMessage.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTMESSAGE_H
#define ASKAP_CP_EVENTCHANNEL_EVENTMESSAGE_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "cms/MapMessage.h"

// Local package includes
#include "eventchannel/IEventMessage.h"

namespace askap {
namespace cp {
namespace eventchannel {

class EventMessage : public IEventMessage {
    public:

        /// @brief Destructor
        virtual ~EventMessage();

        virtual void setBoolean(const std::string& key, const bool val);
        virtual void setChar(const std::string& key, const char val);
        virtual void setBytes(const std::string& key, const std::vector<unsigned char> &val);
        virtual void setShort(const std::string& key, const short val);
        virtual void setInt(const std::string& key, const int val);
        virtual void setLong(const std::string& key, const  long val);
        virtual void setFloat(const std::string& key, const float val);
        virtual void setDouble(const std::string& key, const double val);
        virtual void setString(const std::string& key, const std::string& val);

        virtual bool getBoolean(const std::string& key);
        virtual char getChar(const std::string& key);
        virtual std::vector<unsigned char> getBytes(const std::string& key);
        virtual short getShort(const std::string& key);
        virtual int getInt(const std::string& key);
        virtual long getLong(const std::string& key);
        virtual float getFloat(const std::string& key);
        virtual double getDouble(const std::string& key);
        virtual std::string getString(const std::string& key);

    private:

        /// @brief Constructor
        /// @note EventChannelConnection accesses this constructor as a friend
        EventMessage(cms::MapMessage* message);

        /// @note EventProducer & EventConsumer accesses this method as a friend
        virtual cms::Message* getCmsMessage(void);

        // No support for assignment
        EventMessage& operator=(const EventMessage& rhs);

        // No support for copy constructor
        EventMessage(const EventMessage& src);

        // ActiveMQ CMS message producer
        boost::scoped_ptr<cms::MapMessage> itsMapMessage;

        // Some private functions should have "package" level access. Since
        // C++ does not provide this, use friend to achieve it.
        friend class EventChannelConnection;
        friend class EventProducer;
        friend class EventConsumer;
};

/// Short cut for shared pointer to an EventMessage
typedef boost::shared_ptr<EventMessage> EventMessageSharedPtr;

};
};
};

#endif
