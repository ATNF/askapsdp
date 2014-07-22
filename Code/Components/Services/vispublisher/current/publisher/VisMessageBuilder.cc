/// @file VisMessageBuilder.cc
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
#include "VisMessageBuilder.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes
#include <vector>
#include <complex>
#include <utility>
#include <cmath>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "utils/DelayEstimator.h"

// Local package includes
#include "publisher/VisOutputMessage.h"
#include "publisher/InputMessage.h"

ASKAP_LOGGER(logger, ".VisMessageBuilder");

using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;

VisOutputMessage VisMessageBuilder::build(const InputMessage& in,
        uint32_t tvChanBegin, uint32_t tvChanEnd)
{
    ASKAPCHECK(tvChanEnd >= tvChanBegin, "End chan must be >= start chan");
    const uint32_t nChannel = tvChanEnd - tvChanBegin + 1;
    ASKAPCHECK(nChannel <= in.nChannels(),
            "Number of channels selected exceeds number of channels available");
    const uint32_t nRow = in.nRow();
    const uint32_t nPol = in.nPol();
    const double chanWidth = in.chanWidth();
    const vector< complex<float> >& invis = in.visibilities();
    const vector<uint8_t>& inflag = in.flag();

    VisOutputMessage out;
    out.timestamp() = in.timestamp();
    out.chanBegin() = tvChanBegin;
    out.chanEnd() = tvChanEnd;
    out.data().reserve(nRow * nPol);

    // Process each row, creating nPol VisElements for each row
    vector< complex<float> > vis(nChannel);
    vector<bool> flag(nChannel);
    for (uint32_t row = 0; row < nRow; ++row) {
        for (uint32_t pol = 0; pol < nPol; ++pol) {
            VisElement ve;
            ve.pol = pol;
            ve.beam = in.beam()[row];
            ve.antenna1 = in.antenna1()[row];
            ve.antenna2 = in.antenna2()[row];

            // Build the flag and visibility vectors
            for (uint32_t chan = tvChanBegin; chan <= tvChanEnd; ++chan) {
                const size_t idx = in.index(row, chan, pol);
                vis[chan] = invis[idx];
                flag[chan] = inflag[idx];
            }

            // Calculate the summary statistics
            const std::pair<double, double> ap = ampAndPhase(vis, flag);
            ve.amplitude = ap.first;
            ve.phase = ap.second;
            ve.delay = calcDelay(vis, chanWidth);

            out.data().push_back(ve);
        }
    }

    return out;
}

std::pair<double, double> VisMessageBuilder::ampAndPhase(const std::vector< std::complex<float> >& vis,
                                                       const std::vector<bool>& flag)
{
    ASKAPCHECK(vis.size() == flag.size(), "Vis and Flag vectors not equal size");

    std::complex<double> avg(0,0);
    size_t count = 0;
    for (size_t i = 0; i < vis.size(); ++i) {
        if (!flag[i]) {
            avg += vis[i];
            ++count;
        }
    }
    if (count > 0) {
        avg /= static_cast<double>(count);
    }

    return make_pair(abs(avg), arg(avg) * 180.0 / M_PI);
}

double VisMessageBuilder::calcDelay(const std::vector< std::complex<float> >& vis,
                                    const double chanWidth)
{
    const uint32_t NCHAN_TO_AVG = 54;

    if (vis.size() / NCHAN_TO_AVG < 2) return 0.0;
    ASKAPCHECK(vis.size() % NCHAN_TO_AVG == 0, "Channels to average must divide nChannels");

    askap::scimath::DelayEstimator de(chanWidth * NCHAN_TO_AVG);
    const std::vector< std::complex<float> > avg = averageChannels(vis, NCHAN_TO_AVG);
    return de.getDelay(avg);
}

std::vector< std::complex<float> > VisMessageBuilder::averageChannels(
        const std::vector< std::complex<float> >& vis,
        uint32_t numberToAverage)
{
    ASKAPASSERT(numberToAverage > 0);
    std::vector< std::complex<float> > avg;
    const size_t outputVectorSize = vis.size() / numberToAverage;
    avg.reserve(outputVectorSize);
    for (size_t i = 0; i < outputVectorSize; i += numberToAverage) {
        std::complex<float> a(0.0, 0.0);
        for (size_t j = 0; j < numberToAverage; ++j) {
            a += vis[i + j];
        }
        avg.push_back(a /= numberToAverage);
    }
    return avg;
}
