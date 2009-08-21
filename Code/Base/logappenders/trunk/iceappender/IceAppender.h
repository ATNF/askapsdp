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
#include <map>

// ASKAPsoft includes
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/spi/loggingevent.h>
#include <Ice/Ice.h>

// Local package includes
#include <iceappender/LoggingService.h>

// This is usually frowned upon, but is somewhat necessary for
// the below log4cxx macros. Given nobody actually needs to
// include this file to use the IceAppender (it is self registering)
// this shouldn't cause any real problem.
using namespace log4cxx;

namespace askap {

    /// @brief Implementation of a log4cxx::Appender for remote logging
    /// via Ice.
    ///
    /// @details This class is a self registering appender for the log4cxx
    /// framework. It allows the log events to be sent to a (potentially)
    /// remote logging archiver. However it simply publishes log events on
    /// an IceStorm topic so in practice anything could subscribe to the topic
    /// and receive the messages.
    ///
    /// This appender is designed to be selected by setting up the log4cxx
    /// configuration file appropriatly. Here is an example of a valid configuration
    /// @code
    /// log4j.rootLogger=DEBUG,REMOTE
    ///
    /// log4j.appender.REMOTE=IceAppender
    /// log4j.appender.REMOTE.locator_host=localhost
    /// log4j.appender.REMOTE.locator_port=4061
    /// log4j.appender.REMOTE.topic=logger
    /// @endcode
    ///
    /// This configuration will result in the IceAppender looking for the locator
    /// service (aka registry) on the localhost at port 4061. Log events will
    /// be published to the topic "logger".
    class IceAppender : public log4cxx::AppenderSkeleton {
        public:
            DECLARE_LOG4CXX_OBJECT(IceAppender)
            BEGIN_LOG4CXX_CAST_MAP()
            LOG4CXX_CAST_ENTRY(IceAppender)
            LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
            END_LOG4CXX_CAST_MAP()

            /// @brief Constructor.
            IceAppender();

            /// @brief Destructor.
            virtual ~IceAppender();

            /// @brief This method is called by the AppenderSkeleton#doAppend method.
            /// @detail This is the callback method which will be called when log
            /// events are to be handled by this appender.
            virtual void append(const log4cxx::spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);

            /// @brief This is a callback method called when the appender is closed.
            /// @detail This method is responsible for cleaning up any allocated
            /// resources.
            virtual void close();

            /// @brief Returns true if this appender has been closed,
            /// otherwise false.
            virtual bool isClosed() const;

            /// @brief  Configurators call this method to determine if the
            /// appender requires a layout.
            ///
            /// @return Returns false always, since this appender does not
            ///         need a layout.
            virtual bool requiresLayout() const;

            /// @brief A callback method which is called by the log4cxx framework
            /// to pass configuration options to this class.
            ///
            /// @detail This function is responsible for setting the locator hostname,
            /// locator port number and logger IceStorm topic.
            ///
            /// @param[in]  option  the key (or option).
            /// @param[in]  value the value.
            virtual void setOption(const log4cxx::LogString& option, const log4cxx::LogString& value);

            /// @brief A callback method which is called by the log4cxx framework
            /// when all options have been passed to this class.
            ///
            /// @detail
            /// When this function is called the object should have enough
            /// configuration information to be able to attempt to contact the
            /// Ice locator service.
            ///
            /// @param[in]  pool    not used by this class.
            virtual void activateOptions(log4cxx::helpers::Pool& pool);

            /// @brief Simply utility used to verify if all the options this
            /// class requires have been set.
            ///
            /// @return True if the options have been set, otherwise false.
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

            static std::map<log4cxx::LevelPtr,
                            askap::interfaces::logging::LogLevel> levelMap;

    };

    // Typedef pointer to this class
    typedef helpers::ObjectPtrT<IceAppender> IceAppenderPtr;


} // End namespace

#endif
