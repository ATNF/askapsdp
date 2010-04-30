/// @file TosMetadataAntenna.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "TosMetadataAntenna.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "measures/Measures/MDirection.h"

// Using
using namespace askap::cp;

TosMetadataAntenna::TosMetadataAntenna(const casa::String& name,
                                       const casa::uInt& nCoarseChannels,
                                       const casa::uInt& nBeams,
                                       const casa::uInt& nPol) :
        itsName(name), itsNumCoarseChannels(nCoarseChannels),
        itsNumBeams(nBeams), itsNumPol(nPol), itsFrequency(0.0),
        itsPhaseTrackingCentre(nBeams, nCoarseChannels),
        itsParallacticAngle(0.0), itsOnSource(false),
        itsHwError(true),
        itsFlagDetailed(nBeams, nCoarseChannels, nPol, false),
        itsSystemTemp(nBeams, nCoarseChannels, nPol, -1.0)
{
}

casa::String TosMetadataAntenna::name(void) const
{
    return itsName;
}

casa::uInt TosMetadataAntenna::nCoarseChannels(void) const
{
    return itsNumCoarseChannels;
}

casa::uInt TosMetadataAntenna::nBeams(void) const
{
    return itsNumBeams;
}

casa::uInt TosMetadataAntenna::nPol(void) const
{
    return itsNumPol;
}

casa::MDirection TosMetadataAntenna::dishPointing(void) const
{
    return itsDishPointing;
}

void TosMetadataAntenna::dishPointing(const casa::MDirection& val)
{
    itsDishPointing = val;
}

casa::Double TosMetadataAntenna::frequency(void) const
{
    return itsFrequency;
}

void TosMetadataAntenna::frequency(const casa::Double& val)
{
    itsFrequency = val;
}

casa::String TosMetadataAntenna::clientId(void) const
{
    return itsClientId;
}

void TosMetadataAntenna::clientId(const casa::String& val)
{
    itsClientId = val;
}

casa::String TosMetadataAntenna::scanId(void) const
{
    return itsScanId;
}

void TosMetadataAntenna::scanId(const casa::String& val)
{
    itsScanId = val;
}

casa::MDirection TosMetadataAntenna::phaseTrackingCentre(const casa::uInt& beam,
        const casa::uInt& coarseChannel) const
{
    checkBeam(beam);
    checkCoarseChannel(coarseChannel);
    return itsPhaseTrackingCentre(beam, coarseChannel);
}

void TosMetadataAntenna::phaseTrackingCentre(const casa::MDirection& val,
        const casa::uInt& beam,
        const casa::uInt& coarseChannel)
{
    checkBeam(beam);
    checkCoarseChannel(coarseChannel);
    itsPhaseTrackingCentre(beam, coarseChannel) = val;
}

casa::Double TosMetadataAntenna::parallacticAngle(void) const
{
    return itsParallacticAngle;
}

void TosMetadataAntenna::parallacticAngle(const casa::Double& val)
{
    itsParallacticAngle = val;
}

casa::Bool TosMetadataAntenna::onSource(void) const
{
    return itsOnSource;
}

void TosMetadataAntenna::onSource(const casa::Bool& val)
{
    itsOnSource = val;
}

casa::Bool TosMetadataAntenna::hwError(void) const
{
    return itsHwError;
}

void TosMetadataAntenna::hwError(const casa::Bool& val)
{
    itsHwError = val;
}

casa::Bool TosMetadataAntenna::flagDetailed(const casa::uInt& beam,
        const casa::uInt& coarseChannel,
        const casa::uInt& pol) const
{
    checkBeam(beam);
    checkCoarseChannel(coarseChannel);
    checkPol(pol);
    return itsFlagDetailed(beam, coarseChannel, pol);
}

void TosMetadataAntenna::flagDetailed(const casa::Bool& val,
                                      const casa::uInt& beam,
                                      const casa::uInt& coarseChannel,
                                      const casa::uInt& pol)
{
    checkBeam(beam);
    checkCoarseChannel(coarseChannel);
    checkPol(pol);
    itsFlagDetailed(beam, coarseChannel, pol) = val;
}

casa::Float TosMetadataAntenna::systemTemp(const casa::uInt& beam,
        const casa::uInt& coarseChannel,
        const casa::uInt& pol) const
{
    checkBeam(beam);
    checkCoarseChannel(coarseChannel);
    checkPol(pol);
    return itsSystemTemp(beam, coarseChannel, pol);
}

void TosMetadataAntenna::systemTemp(const casa::Float& val,
                                    const casa::uInt& beam,
                                    const casa::uInt& coarseChannel,
                                    const casa::uInt& pol)
{
    checkBeam(beam);
    checkCoarseChannel(coarseChannel);
    checkPol(pol);
    itsSystemTemp(beam, coarseChannel, pol) = val;
}

void TosMetadataAntenna::checkBeam(const casa::uInt& beam) const
{
    if (beam > (itsNumBeams - 1)) {
        ASKAPTHROW(AskapError, "Invalid beam index");
    }
}

void TosMetadataAntenna::checkCoarseChannel(const casa::uInt& coarseChannel) const
{
    if (coarseChannel > (itsNumCoarseChannels - 1)) {
        ASKAPTHROW(AskapError, "Invalid coarse channel index");
    }
}

void TosMetadataAntenna::checkPol(const casa::uInt& pol) const
{
    if (pol > (itsNumPol - 1)) {
        ASKAPTHROW(AskapError, "Invalid pol index");
    }
}
