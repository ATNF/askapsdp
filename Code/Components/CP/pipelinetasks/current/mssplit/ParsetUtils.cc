/// @file ParsetUtils.cc
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

// Package level header file
#include "askap_pipelinetasks.h"

// Include own header file
#include "mssplit/ParsetUtils.h"

// System includes
#include <string>
#include <utility>
#include <algorithm>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/AskapUtil.h"
#include "boost/shared_ptr.hpp"
#include "boost/regex.hpp"
#include "Common/ParameterSet.h"

ASKAP_LOGGER(logger, ".parsetutils");

using namespace askap::cp::pipelinetasks;
using namespace std;

bool ParsetUtils::isInteger(const std::string& val)
{
    boost::smatch what;
    const boost::regex e("[\\d]+");
    return regex_match(val, what, e);
}

bool ParsetUtils::isRange(const std::string& val)
{
    boost::smatch what;
    const boost::regex e("([\\d]+)\\s*-\\s*([\\d]+)");
    return regex_match(val, what, e);
}

std::pair<uint32_t, uint32_t> ParsetUtils::parseIntRange(
    const LOFAR::ParameterSet& parset,
    const std::string& key)
{
    std::pair<uint32_t, uint32_t> result;
    const std::string raw = parset.getString(key);

    // These are the two possible patterns, either a single integer, or an
    // integer separated by a dash (potentially surrounded by whitespace)
    if (isInteger(raw)) {
        result.first = utility::fromString<uint32_t>(raw);
        result.second = result.first;
    } else if (isRange(raw)) {
        // Now extract the first and second integer
        const boost::regex digits("[\\d]+");
        boost::sregex_iterator it(raw.begin(), raw.end(), digits);
        boost::sregex_iterator end;

        result.first = utility::fromString<uint32_t>(it->str());
        ++it;
        result.second = utility::fromString<uint32_t>(it->str());
    } else {
        ASKAPLOG_ERROR_STR(logger, "Invalid format '" << key << "' parameter");
    }

    return result;
}
