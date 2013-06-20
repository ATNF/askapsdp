/// @file ChannelSelTask.h
///
///
/// Selection of subset of channels
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// Include own header file first
#include "ChannelSelTask.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Cube.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "configuration/Configuration.h"

ASKAP_LOGGER(logger, ".ChannelSelTask");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

ChannelSelTask::ChannelSelTask(const LOFAR::ParameterSet& parset,
        const Configuration& config) :
    itsParset(parset)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    ASKAPCHECK(config.nprocs() == 1, "The current implementation of channel selection task works in the standalone mode only");
    itsStart = parset.getUint32("start");
    itsNChan = parset.getUint32("nchan");
}

ChannelSelTask::~ChannelSelTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
}

void ChannelSelTask::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");
    ASKAPDEBUGASSERT(chunk);
    const casa::uInt nChanOriginal = chunk->nChannel();

    if (itsStart + itsNChan > nChanOriginal) {
        ASKAPLOG_WARN_STR(logger, "Channel selection task got chunk with "<<nChanOriginal<<
                 " channels, unable to select "<<itsNChan<<" channels starting from "<<itsStart);
        chunk->flag().set(true);
        return;
    }

    // extract required frequencies
    // don't take const reference to be able to take slice (although we don't change the chunk yet)
    casa::Vector<casa::Double>& origFreq = chunk->frequency();
    casa::Vector<casa::Double> newFreq = origFreq(casa::Slice(itsStart,itsNChan));
    ASKAPDEBUGASSERT(newFreq.nelements() == itsNChan);

    // Extract slices from vis and flag cubes
    const casa::uInt nRow = chunk->nRow();
    const casa::uInt nPol = chunk->nPol();
    casa::Cube<casa::Complex>& origVis = chunk->visibility();
    casa::Cube<casa::Bool>& origFlag = chunk->flag();
    const casa::IPosition start(3, 0, itsStart, 0);
    const casa::IPosition length(3, nRow, itsNChan, nPol);
    const casa::Slicer slicer(start,length);

    casa::Cube<casa::Complex> newVis = origVis(slicer);
    casa::Cube<casa::Bool> newFlag = origFlag(slicer);
    ASKAPDEBUGASSERT(newVis.shape() == length);
    ASKAPDEBUGASSERT(newFlag.shape() == length);

    chunk->resize(newVis, newFlag, newFreq);
}
