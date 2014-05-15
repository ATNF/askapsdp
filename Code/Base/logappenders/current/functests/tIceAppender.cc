/// @file tIceAppender.cc
///
/// @detail
/// This test simply sends a single message via the IceAppender and ensures
/// it has been received. This test program acts as the Log Archiver, which
/// usually subscribes to he appropriate IceStorm topic and receives the
/// LogEvents.
///
/// The test requires the presence of this file "tIceAppender.log_cfg" which
/// would usually have the following contents:
///
/// @code
/// log4j.rootLogger=DEBUG,REMOTE
///
/// log4j.appender.REMOTE=IceAppender
/// log4j.appender.REMOTE.locator_host=localhost
/// log4j.appender.REMOTE.locator_port=4061
/// log4j.appender.REMOTE.topic=logger
/// @endcode
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

// System includes
#include <string>
#include <iostream>

// Log4cxx includes
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/mdc.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/fileappender.h>

// Local package includes
#include <askap_logappenders.h>

// Using
using namespace log4cxx;

// Test input and output
static const std::string inputMessage = "Testing the IceAppender";
static const std::string inputLogname = "MyLogger";


int main(int argc, char *argv[])
{
    const std::string filename = "tIceAppender.log_cfg";

    // Configure the local logger
    log4cxx::PropertyConfigurator::configure(log4cxx::File(filename));
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(inputLogname);

    // Send the test log message
    LOG4CXX_INFO(logger, inputMessage);
    return 0;

}
