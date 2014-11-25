/// @file SpectralIndex.cc
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

// Include own header file first
#include "SpectralIndex.h"

// Include package level header file
#include "askap_components.h"

// System includes
#include <cmath>

// ASKAPsoft includes
#include "askap/AskapError.h"

// Local package includes
#include "SpectralModel.h"
#include "ComponentType.h"

using namespace askap;
using namespace askap::components;

SpectralIndex::SpectralIndex(const casa::MFrequency& refFreq, double index)
    : itsReferenceFreq(refFreq), itsSpectralIndex(index)
{
    ASKAPCHECK(refFreq.get("Hz").getValue() > 0.0,
            "Reference frequency is zero or negative");
}

ComponentType::SpectralShape SpectralIndex::type(void) const
{
    return ComponentType::SPECTRAL_INDEX;
}

double SpectralIndex::sample(const casa::MFrequency& centerFrequency) const
{
    ASKAPCHECK(centerFrequency.type() == itsReferenceFreq.type(),
            "User frequency and reference frequency have differing frames");
    ASKAPCHECK(centerFrequency.get("Hz").getValue() > 0.0,
            "User frequency is zero or negative");
    return std::pow(centerFrequency.get("Hz").getValue() / itsReferenceFreq.get("Hz").getValue(),
            itsSpectralIndex);
}

const casa::MFrequency& SpectralIndex::getRefFreq(void) const
{
    return itsReferenceFreq;
}

double SpectralIndex::getIndex(void) const
{
    return itsSpectralIndex;
}
