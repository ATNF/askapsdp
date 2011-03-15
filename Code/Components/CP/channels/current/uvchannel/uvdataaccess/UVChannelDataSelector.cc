/// @file UVChannelDataSelector.cc
/// @brief
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
#include "UVChannelDataSelector.h"

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "dataaccess/DataAccessError.h"

// Using
using namespace askap;
using namespace askap::accessors;
using namespace askap::cp::channels;
using namespace casa;

UVChannelDataSelector::UVChannelDataSelector()
{
}

void UVChannelDataSelector::chooseFeed(casa::uInt feedID)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseBaseline(casa::uInt ant1, casa::uInt ant2)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseAutoCorrelations()
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseCrossCorrelations()
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseMinUVDistance(casa::Double uvDist)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseMaxUVDistance(casa::Double uvDist)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseChannels(casa::uInt nChan,
        casa::uInt start, casa::uInt nAvg)
{
    if (nAvg != 1) {
        throw DataAccessLogicError("Channel averaging is not yet implemented");
    }

    if (nChan != 1) {
        throw DataAccessLogicError("Only supports single channel");
    }

    ASKAPDEBUGASSERT((nChan > 0) && (start >= 0));
    itsChannelSelection.first = nChan;
    itsChannelSelection.second = start;
}

void UVChannelDataSelector::chooseFrequencies(casa::uInt nChan,
        const casa::MVFrequency &start,
        const casa::MVFrequency &freqInc)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseVelocities(casa::uInt nChan,
        const casa::MVRadialVelocity &start,
        const casa::MVRadialVelocity &velInc)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseSpectralWindow(casa::uInt spWinID)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseTimeRange(const casa::MVEpoch &start,
        const casa::MVEpoch &stop)
{
    throw DataAccessLogicError("Not applicable for streaming accessor");
}

void UVChannelDataSelector::chooseTimeRange(casa::Double start, casa::Double stop)
{
    throw DataAccessLogicError("Not applicable for streaming accessor");
}

void UVChannelDataSelector::choosePolarizations(const casa::String &pols)
{
    throw DataAccessLogicError("not yet implemented");
}

void UVChannelDataSelector::chooseCycles(casa::uInt start, casa::uInt stop)
{
    throw DataAccessLogicError("Not applicable for streaming accessor");
}

bool UVChannelDataSelector::channelsSelected() const
{
    return (itsChannelSelection.first >= 0);
}

std::pair<casa::uInt, casa::uInt> UVChannelDataSelector::getChannelSelection() const
{
    return itsChannelSelection;
}
