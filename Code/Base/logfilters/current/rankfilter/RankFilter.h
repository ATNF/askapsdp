/// @file RankFilter.h
///
/// @copyright (c) 2013 CSIRO
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

#ifndef ASKAP_RANKFILTER_H
#define ASKAP_RANKFILTER_H

// System includes
#include <string>

// ASKAPsoft includes
#include <log4cxx/spi/filter.h>
#include <log4cxx/level.h>

// This is usually frowned upon, but is somewhat necessary for
// the below log4cxx macros. Given nobody actually needs to
// include this file to use the IceAppender (it is self registering)
// this shouldn't cause any real problem.
using namespace log4cxx;

namespace askap {
    /// This filter is based on matching the "mpirank" key/value pair
    /// within the MDC
    ///
    /// The filter admits two options RankToMatch and AcceptOnMatch. If there is
    /// an exact match between the value of the RankToMatch option with the "mpirank"
    /// MDC value then the log message is ACCEPTED in the case the option AcceptOnMatch
    /// is set to true. If it is false then the logg message is DENIED.
    ///
    /// If there is no match between the RankToMatch option with the "mpirank" MDC
    /// value then opposite of the above occurs.
    ///
    /// If the RankToMatch is not set or the MDC has no "mpirank" entry then
    /// NEUTRAL is returned from decide.
    ///
    /// The AcceptOnMatch option has a default value of true.
    class LOG4CXX_EXPORT RankFilter : public log4cxx::spi::Filter
    {

        public:
            typedef log4cxx::spi::Filter BASE_CLASS;

            DECLARE_LOG4CXX_OBJECT(RankFilter)
                BEGIN_LOG4CXX_CAST_MAP()
                LOG4CXX_CAST_ENTRY(RankFilter)
                LOG4CXX_CAST_ENTRY_CHAIN(BASE_CLASS)
                END_LOG4CXX_CAST_MAP()

            /// Constructor
            RankFilter();

            /// Called by log4cxx framework to set options
            virtual void setOption(const LogString& option,
                                   const LogString& value);

            ///  Return the decision of this filter.
            FilterDecision decide(const spi::LoggingEventPtr& event) const;

        private:
            // The "AcceptOnMatch" option
            bool itsAcceptOnMatch;

            // The "RankToMatch" option
            std::string itsRank;
    };

    LOG4CXX_PTR_DEF(RankFilter);
}

#endif
