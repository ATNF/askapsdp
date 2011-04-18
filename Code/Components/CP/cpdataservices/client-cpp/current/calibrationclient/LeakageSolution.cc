/// @file LeakageSolution.cc
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
#include "LeakageSolution.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

// Using
using namespace askap::cp::caldataservice;

LeakageSolution::LeakageSolution(const casa::Long timestamp,
                                 const casa::Short nAntenna,
                                 const casa::Short nBeam)
        : itsTimestamp(timestamp), itsNAntenna(nAntenna), itsNBeam(nBeam),
        itsLeakage(nAntenna, nBeam),
        itsAntennaIndex(nAntenna),
        itsBeamIndex(nBeam)
{
    casa::DComplex c(-1.0);
    itsLeakage = c;
    itsAntennaIndex = 0;
    itsBeamIndex = 0;
}

casa::Long LeakageSolution::timestamp(void) const
{
        return itsTimestamp;
}

casa::Short LeakageSolution::nAntenna(void) const
{
        return itsNAntenna;
}

casa::Short LeakageSolution::nBeam(void) const
{
        return itsNBeam;
}

const casa::Matrix<casa::DComplex>& LeakageSolution::leakage(void) const
{
    return itsLeakage;
}

casa::Matrix<casa::DComplex>& LeakageSolution::leakage(void)
{
    return itsLeakage;
}

const casa::Vector<casa::Int>& LeakageSolution::antennaIndex(void) const
{
    return itsAntennaIndex;
}

casa::Vector<casa::Int>& LeakageSolution::antennaIndex(void)
{
    return itsAntennaIndex;
}

const casa::Vector<casa::Int>& LeakageSolution::beamIndex(void) const
{
    return itsBeamIndex;
}

casa::Vector<casa::Int>& LeakageSolution::beamIndex(void)
{
    return itsBeamIndex;
}

