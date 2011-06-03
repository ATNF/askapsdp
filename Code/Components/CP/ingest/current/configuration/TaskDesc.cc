/// @file TaskDesc.cc
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
#include "TaskDesc.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"

using namespace std;
using namespace askap;
using namespace askap::cp::ingest;

TaskDesc::TaskDesc(const std::string& name,
        const TaskDesc::Type type,
        const LOFAR::ParameterSet& parset)
    : itsName(name), itsType(type), itsParset(parset)
{
}

std::string TaskDesc::name(void) const
{
    return itsName;
}

TaskDesc::Type TaskDesc::type(void) const
{
    return itsType;
}

LOFAR::ParameterSet TaskDesc::parset(void) const
{
    return itsParset;
}
