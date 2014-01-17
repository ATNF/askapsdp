/// @file IceAppender.cc
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
#include <rankfilter/RankFilter.h>

// ASKAPsoft includes
#include <log4cxx/logstring.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>

using namespace askap;
using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(RankFilter)

RankFilter::RankFilter() : itsAcceptOnMatch(true)
{
}

void RankFilter::setOption(const LogString& option, const LogString& value)
{
    if (StringHelper::equalsIgnoreCase(option,
                LOG4CXX_STR("RANKTOMATCH"), LOG4CXX_STR("ranktomatch")))
    {
        itsRank = value;
    } else if (StringHelper::equalsIgnoreCase(option,
                LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
    {
        itsAcceptOnMatch = OptionConverter::toBoolean(value, itsAcceptOnMatch);
    }
}

Filter::FilterDecision RankFilter::decide(const log4cxx::spi::LoggingEventPtr& event) const
{
    LogString val;

    // RankToAccept not set or no MDC "mpirank"
    if (itsRank.empty() || !event->getMDC("mpirank", val)) return Filter::NEUTRAL;

    if (itsRank == val) {
        if (itsAcceptOnMatch) {
            return Filter::ACCEPT;
        } else {
            return Filter::DENY;
        }
    } else {
        if (itsAcceptOnMatch) {
            return Filter::DENY;
        } else {
            return Filter::ACCEPT;
        }
    }
}
