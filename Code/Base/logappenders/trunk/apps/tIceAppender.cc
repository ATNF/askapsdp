/// @file tIceAppender.cc
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

#include <askap_logappenders.h>

#include <string>
#include <iostream>

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/mdc.h>

#include <log4cxx/simplelayout.h>
#include <log4cxx/fileappender.h>


#include <iceappender/IceAppender.h>

using namespace log4cxx;

int main(int argc, char *argv[])
{
    const std::string filename = "tIceAppender.log_cfg";

    log4cxx::PropertyConfigurator::configure(log4cxx::File(filename));

    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("MyLogger");

    askap::IceAppenderPtr appender = new askap::IceAppender;
    //logger->addAppender(appender);

    LOG4CXX_INFO(logger, "Trying out the Apache log4cxx API");

    return 0;
}
