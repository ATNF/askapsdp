/// @file ParsetUtils.h
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

#ifndef ASKAP_CP_PARSETUTILS_H
#define ASKAP_CP_PARSETUTILS_H

// Package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <utility>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

class ParsetUtils {
    public:

        /// Returns true if the string can be parsed as an integer
        static bool isInteger(const std::string& val);

        /// Returns true if the string can be parsed as a range (of integers)
        /// of the form:
        /// "1 - 300"
        static bool isRange(const std::string& val);

        /// Returns a pair specifying a range. Some examples follow:
        /// "1 - 300" -> (1, 300)
        /// "1-300" -> (1, 300)
        /// "1" -> (1, 1)
        static std::pair<unsigned int, unsigned int> parseIntRange(
            const LOFAR::ParameterSet& parset,
            const std::string& key);
};

}
}
}
#endif
