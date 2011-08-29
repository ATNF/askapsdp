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

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/Quanta/Quantum.h"

// Using
using namespace askap::cp::skymodelservice;

Component::Component(const ComponentId id,
        const casa::Quantity& rightAscension,
        const casa::Quantity& declination,
        const casa::Quantity& positionAngle,
        const casa::Quantity& majorAxis,
        const casa::Quantity& minorAxis,
        const casa::Quantity& i1400,
        const casa::Double& spectralIndex)
    : itsId(id),
    itsRightAscension(rightAscension),
    itsDeclination(declination),
    itsPositionAngle(positionAngle),
    itsMajorAxis(majorAxis),
    itsMinorAxis(minorAxis),
    itsI1400(i1400),
    itsSpectralIndex(spectralIndex)
{
    ASKAPCHECK(itsRightAscension.isConform("deg"), "ra must conform to degrees");
    ASKAPCHECK(itsDeclination.isConform("deg"), "dec must conform to degrees");
    ASKAPCHECK(itsPositionAngle.isConform("rad"), "position angle must conform to radians");
    ASKAPCHECK(itsMajorAxis.isConform("arcsec"), "major axis must conform to degrees");
    ASKAPCHECK(itsMinorAxis.isConform("arcsec"), "minor axis must conform to degrees");
    ASKAPCHECK(itsI1400.isConform("Jy"), "i1400 must conform to Jy");
}

ComponentId Component::id() const
{
    return itsId;
}

casa::Quantity Component::rightAscension() const
{
    return itsRightAscension;
}

casa::Quantity Component::declination() const
{
    return itsDeclination;
}

casa::Quantity Component::positionAngle() const
{
    return itsPositionAngle;
}

casa::Quantity Component::majorAxis() const
{
    return itsMajorAxis;
}

casa::Quantity Component::minorAxis() const
{
    return itsMinorAxis;
}

casa::Quantity Component::i1400() const
{
    return itsI1400;
}

casa::Double Component::spectralIndex() const
{
    return itsSpectralIndex;
}
