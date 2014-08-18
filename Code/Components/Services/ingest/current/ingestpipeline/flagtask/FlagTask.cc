/// @file FlagTask.cc
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
/// @author Ben Humphreys

// Include package level header file
#include "askap_cpingest.h"

// Include own header file
#include "ingestpipeline/flagtask/FlagTask.h"

// System includes
#include <limits>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"

ASKAP_LOGGER(logger, ".FlagTask");

using namespace std;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

FlagTask::FlagTask(const LOFAR::ParameterSet& parset, const Configuration& config)
{
    const string CROSS_THRESHOLD_KEY = "threshold.crosscorr";
    itsCrossCorrThresholdSet = parset.isDefined(CROSS_THRESHOLD_KEY);
    itsCrossCorrThreshold = parset.getFloat(CROSS_THRESHOLD_KEY,
            numeric_limits<float>::max());
    if (itsCrossCorrThresholdSet) {
        ASKAPLOG_INFO_STR(logger,
                "Amplitude threshold set for cross-correlations: "
                << itsCrossCorrThreshold);
    }

    const string AUTO_THRESHOLD_KEY = "threshold.autocorr";
    itsAutoCorrThresholdSet = parset.isDefined(AUTO_THRESHOLD_KEY);
    itsAutoCorrThreshold =  parset.getFloat(AUTO_THRESHOLD_KEY,
            numeric_limits<float>::max());
    if (itsAutoCorrThresholdSet) {
        ASKAPLOG_INFO_STR(logger,
                "Amplitude threshold set for auto-correlations: "
                << itsAutoCorrThreshold);
    }

    itsZeroFlagged = parset.getBool("zeroflagged", false);
    if (itsZeroFlagged) {
        ASKAPLOG_INFO_STR(logger,
                "Visibilities exceeding the defined thresholds will be set to zero");
    }
}

FlagTask::~FlagTask()
{
}

void FlagTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
    if (!itsCrossCorrThresholdSet && !itsAutoCorrThresholdSet) {
        return;
    }

    const casa::uInt nRow = chunk->nRow();
    const casa::uInt nChannel = chunk->nChannel();
    const casa::uInt nPol = chunk->nPol();

    casa::Cube<casa::Complex>& vis = chunk->visibility();
    casa::Cube<casa::Bool>& flag = chunk->flag();
    const casa::Vector<casa::uInt>& ant1 = chunk->antenna1();
    const casa::Vector<casa::uInt>& ant2 = chunk->antenna2();

    for (casa::uInt pol = 0; pol < nPol; ++pol) {
        for (casa::uInt chan = 0; chan < nChannel; ++chan) {
            for (casa::uInt row = 0; row < nRow; ++row) {
                const bool isAuto = ant1(row) == ant2(row);
                if (isAuto && !itsAutoCorrThresholdSet) continue;
                if (!isAuto && !itsCrossCorrThresholdSet) continue;

                if (flag(row, chan, pol)) continue;
                const float amp = abs(vis(row, chan, pol));

                if ((isAuto && amp > itsAutoCorrThreshold)
                        || (!isAuto && amp > itsCrossCorrThreshold)) {
                    flag(row, chan, pol) = true;
                    if (itsZeroFlagged) vis(row, chan, pol) = 0.0;
                }
            }
        }

    }
}
