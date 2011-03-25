/// @file Component.cc
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
#include "Component.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"

// Using
using namespace std;
using namespace askap::cp::skymodelservice;

Component::Component(const ComponentId id,
        double rightAscension,
        double declination,
        double positionAngle,
        double majorAxis,
        double minorAxis,
        double i1400)
    : itsId(id),
    itsRightAscension(rightAscension),
    itsDeclination(declination),
    itsPositionAngle(positionAngle),
    itsMajorAxis(majorAxis),
    itsMinorAxis(minorAxis),
    itsI1400(i1400)
{
}

ComponentId Component::id() const
{
    return itsId;
}

double Component::rightAscension() const
{
    return itsRightAscension;
}

double Component::declination() const
{
    return itsDeclination;
}

double Component::positionAngle() const
{
    return itsPositionAngle;
}

double Component::majorAxis() const
{
    return itsMajorAxis;
}

double Component::minorAxis() const
{
    return itsMinorAxis;
}

double Component::i1400() const
{
    return itsI1400;
}
