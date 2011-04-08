/// @file BandpassSolution.cc
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
#include "BandpassSolution.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "casa/Arrays/Cube.h"
#include "casa/Arrays/Vector.h"
#include "calibrationclient/JonesJTerm.h"

// Local package includes

// Using
using namespace askap::cp::caldataservice;

BandpassSolution::BandpassSolution(const casa::Long timestamp,
                                   const casa::Short nAntenna,
                                   const casa::Short nBeam,
                                   const casa::Int nChan)
        : itsTimestamp(timestamp), itsNAntenna(nAntenna), itsNBeam(nBeam), itsNChan(nChan),
        itsBandpass(nAntenna, nBeam, nChan),
        itsAntennaIndex(nAntenna),
        itsBeamIndex(nBeam),
        itsChanIndex(nChan)
{
}


const casa::Cube<JonesJTerm>& BandpassSolution::bandpass(void) const
{
    return itsBandpass;
}

casa::Cube<JonesJTerm>& BandpassSolution::bandpass(void)
{
    return itsBandpass;
}

const casa::Vector<casa::Int>& BandpassSolution::antennaIndex(void) const
{
    return itsAntennaIndex;
}

casa::Vector<casa::Int>& BandpassSolution::antennaIndex(void)
{
    return itsAntennaIndex;
}

const casa::Vector<casa::Int>& BandpassSolution::beamIndex(void) const
{
    return itsBeamIndex;
}

casa::Vector<casa::Int>& BandpassSolution::beamIndex(void)
{
    return itsBeamIndex;
}

const casa::Vector<casa::Int>& BandpassSolution::chanIndex(void) const
{
    return itsChanIndex;
}

casa::Vector<casa::Int>& BandpassSolution::chanIndex(void)
{
    return itsChanIndex;
}

