/// @file ChannelAvgTask.cc
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
#include "ChannelAvgTask.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Cube.h"

// Local package includes
#include "ingestpipeline/datadef/VisChunk.h"

ASKAP_LOGGER(logger, ".ChannelAvgTask");

using namespace askap;
using namespace askap::cp;

ChannelAvgTask::ChannelAvgTask(const LOFAR::ParameterSet& parset) :
    itsParset(parset)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    itsAveraging = parset.getUint32("averaging");
}

ChannelAvgTask::~ChannelAvgTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
}

void ChannelAvgTask::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");
    if (itsAveraging < 2) {
        // No averaging required for 0 or 1
        return;
    }

    const casa::uInt nChanOriginal = chunk->nChannel();
    if (nChanOriginal % itsAveraging != 0) {
        ASKAPTHROW(AskapError, "Number of channels not a multiple of averaging number");
    }
    const casa::uInt nChanNew = nChanOriginal / itsAveraging;

    // Average frequencies vector
    const casa::Vector<casa::Double>& origFreq = chunk->frequency();
    casa::Vector<casa::Double> newFreq(nChanNew);

    for (casa::uInt newIdx = 0; newIdx < nChanNew; ++newIdx) {
        const casa::uInt origIdx = itsAveraging * newIdx;
        casa::Double sum = 0.0;
        for (casa::uInt i = 0; i < itsAveraging; ++i) {
            sum += origFreq(origIdx + i);
        }
        newFreq(newIdx) = sum / itsAveraging;
    }

    // Average vis and flag cubes
    const casa::uInt nRow = chunk->nRow();
    const casa::uInt nPol = chunk->nPol();
    const casa::Cube<casa::Complex>& origVis = chunk->visibility();
    const casa::Cube<casa::Bool>& origFlag = chunk->flag();
    casa::Cube<casa::Complex> newVis(nRow, nChanNew, nPol);
    casa::Cube<casa::Bool> newFlag(nRow, nChanNew, nPol);

    for (casa::uInt row = 0; row < nRow; ++row) {
        for (casa::uInt newIdx = 0; newIdx < nChanNew; ++newIdx) {
            for (casa::uInt pol = 0; pol < nPol; ++pol) {

                // Track the samples added, since those flagged are not
                casa::uInt numGoodSamples = 0;

                // Calculate the average over the number of samples to
                // be averaged together (itsAveraging)
                const casa::uInt origIdx = itsAveraging * newIdx;
                casa::Complex sum(0.0, 0.0);
                for (casa::uInt i = 0; i < itsAveraging; ++i) {
                    // Only sum if not flagged
                    if (!origFlag(row, origIdx + i, pol)) {
                        sum += origVis(row, origIdx + i, pol);
                        numGoodSamples++;
                    }
                }
                casa::Complex avg(sum.real() / numGoodSamples,
                        sum.imag() / numGoodSamples);
                newVis(row, newIdx, pol) = avg;

                // Flag if there were no samples to average
                if (numGoodSamples > 0) {
                    newFlag(row, newIdx, pol) = false;
                } else {
                    newFlag(row, newIdx, pol) = true;
                }
            }
        }
    }

    chunk->resize(newVis, newFlag, newFreq);
}
