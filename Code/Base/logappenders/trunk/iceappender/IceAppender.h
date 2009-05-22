/// @file IceAppender.h
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

#ifndef ASKAP_ICEAPPENDER_H
#define ASKAP_ICEAPPENDER_H

// System includes
#include <string>

// ASKAPsoft includes
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/spi/loggingevent.h>
#include <Ice/Ice.h>

// Local package includes
#include <iceappender/LoggingService.h>

// This is usually frowned upon, but is somewhat necessary for
// the below log4cxx macros.
using namespace log4cxx;

namespace askap
{
        class IceAppender : public log4cxx::AppenderSkeleton
        {
        public:
                DECLARE_LOG4CXX_OBJECT(IceAppender)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(IceAppender)
                        LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                END_LOG4CXX_CAST_MAP()

                IceAppender();
                virtual ~IceAppender();

                /**
                This method is called by the AppenderSkeleton#doAppend
                method.
                */
                virtual void append(const log4cxx::spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);

                virtual void close();

                virtual bool isClosed() const;

                virtual bool requiresLayout() const;

                virtual void activateOptions(log4cxx::helpers::Pool& pool);
                virtual void setOption(const log4cxx::LogString& option, const log4cxx::LogString& value);

                virtual bool verifyOptions() const;
        private:
                // The ICE communicator
                Ice::CommunicatorPtr itsIceComm;

                // Proxy to the log service
                askap::interfaces::logging::ILoggerPrx itsLogService;

                // Paramater - The hostname of the locator service
                std::string itsLocatorHost;

                // Paramater - The port number of the locator service
                std::string itsLocatorPort;

                // Paramater - The topic to which log messages will be sent
                std::string itsLoggingTopic;
        };

        typedef helpers::ObjectPtrT<IceAppender> IceAppenderPtr;

} // End namespace

#endif
