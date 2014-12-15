/// @file TypedValueMapper.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_CP_ICEWRAPPER_TYPEDVALUEMAPPER_H
#define ASKAP_CP_ICEWRAPPER_TYPEDVALUEMAPPER_H

// System includes
#include <string>
#include <stdint.h>

// ASKAPsoft includes
#include "Ice/Ice.h"

// Ice interfaces includes
#include "TypedValues.h"

namespace askap {
namespace cp {
namespace icewrapper {

/// @brief Utility class providing functions to convert native types
/// to Ice TypedValue types.
///
/// This is purposefully using overloaded functions, however one does need
/// to be careful of type conversions.
class TypedValueMapper {
    public:
        static askap::interfaces::TypedValuePtr toTypedValue(int value);

        static askap::interfaces::TypedValuePtr toTypedValue(long value);

        static askap::interfaces::TypedValuePtr toTypedValue(uint32_t value);

        static askap::interfaces::TypedValuePtr toTypedValue(uint64_t value);

        static askap::interfaces::TypedValuePtr toTypedValue(float value);

        static askap::interfaces::TypedValuePtr toTypedValue(double value);

        static askap::interfaces::TypedValuePtr toTypedValue(bool value);

        static askap::interfaces::TypedValuePtr toTypedValue(const std::string& value);
};

}
}
}

#endif
