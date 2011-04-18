/// @file GainSolution.cc
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
#include "GainSolution.h"

// Include package level header file
#include "askap_cpdataservices.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

// Local package includes
#include "calibrationclient/JonesJTerm.h"

// Using
using namespace std;
using namespace askap::cp::caldataservice;

GainSolution::GainSolution(const casa::Long timestamp,
                           const casa::Short nAntenna,
                           const casa::Short nBeam)
        : itsTimestamp(timestamp), itsNAntenna(nAntenna), itsNBeam(nBeam),
        itsGains(nAntenna, nBeam),
        itsAntennaIndex(nAntenna),
        itsBeamIndex(nBeam)
{
    JonesJTerm jterm;
    itsGains = jterm;
    itsAntennaIndex = 0;
    itsBeamIndex = 0;
}

casa::Long GainSolution::timestamp(void) const
{
    return itsTimestamp;
}

casa::Short GainSolution::nAntenna(void) const
{
    return itsNAntenna;
}

casa::Short GainSolution::nBeam(void) const
{
    return itsNBeam;
}

const casa::Matrix<JonesJTerm>& GainSolution::gains(void) const
{
    return itsGains;
}

casa::Matrix<JonesJTerm>& GainSolution::gains(void)
{
    return itsGains;
}

const casa::Vector<casa::Int>& GainSolution::antennaIndex(void) const
{
    return itsAntennaIndex;
}

casa::Vector<casa::Int>& GainSolution::antennaIndex(void)
{
    return itsAntennaIndex;
}

const casa::Vector<casa::Int>& GainSolution::beamIndex(void) const
{
    return itsBeamIndex;
}

casa::Vector<casa::Int>& GainSolution::beamIndex(void)
{
    return itsBeamIndex;
}
