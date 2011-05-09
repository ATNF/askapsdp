/// @file ParsetUtils.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "cmodel/ParsetUtils.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes

// ASKAPsoft include
#include "askap/AskapError.h"
#include "casa/aipstype.h"

// Using
using namespace askap::cp::pipelinetasks;

casa::MDirection ParsetUtils::asMDirection(const std::vector<std::string>& direction)
{
    ASKAPCHECK(direction.size()==3, "Not a valid direction");

    casa::Quantity lng;
    casa::Quantity::read(lng, direction[0]);
    casa::Quantity lat;
    casa::Quantity::read(lat, direction[1]);
    casa::MDirection::Types type;
    casa::MDirection::getType(type, direction[2]);
    casa::MDirection dir(lng, lat, type);
    return dir;
}

casa::Quantum<casa::Double> ParsetUtils::createQuantity(const std::string &strval,
        const std::string &unit)
{
    casa::Quantity q;
    casa::Quantity::read(q, strval);
    return q;
}
