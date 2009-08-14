/// @file Activity.cc
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

// Include own header file first
#include "Activity.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"

// Using
using namespace askap;
using askap::cp::Activity;

ASKAP_LOGGER(logger, ".Activity");

Activity::Activity() : itsStopRequested(false)
{
}

Activity::~Activity()
{
}

void Activity::start(void)
{
    if (itsThread) {
        ASKAPTHROW(AskapError, "Thread has already been started");
    }
    ASKAPLOG_INFO_STR(logger, "Starting thread for activity " << getName());
    itsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Activity::run, this)));
    itsStopRequested = false;
}

void Activity::stop(void)
{
    ASKAPLOG_INFO_STR(logger, "Stopping thread for activity " << getName());
    if (!itsThread) {
        ASKAPTHROW(AskapError, "Thread is not running");
    }
    itsStopRequested = true;
    itsThread->join();
    itsThread.reset();
}

bool Activity::stopRequested(void)
{
    return itsStopRequested;
}

std::string Activity::getName(void)
{
    return itsName;
}

void Activity::setName(const std::string& name)
{
    itsName = name;
}
