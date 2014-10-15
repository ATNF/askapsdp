/// @file SubsetExtractor.cc
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
#include "SubsetExtractor.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

// Local package includes
#include "publisher/SpdOutputMessage.h"
#include "publisher/InputMessage.h"

ASKAP_LOGGER(logger, ".SubsetExtractor");

using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;

SpdOutputMessage SubsetExtractor::subset(const InputMessage& in, uint32_t beam, uint32_t pol)
{
    const uint32_t nRow = in.nRow();
    const uint32_t nChannels = in.nChannels();
    const uint32_t nPols = in.nPol();

    SpdOutputMessage out;
    out.timestamp() = in.timestamp();
    out.scan() = in.scan();
    out.beamId() = beam;
    out.polId() = pol;
    out.nChannels() = nChannels;
    out.chanWidth() = in.chanWidth();
    out.frequency() = in.frequency();

    // Make antenna vectors
    vector<uint32_t> ant1;
    vector<uint32_t> ant2;
    const uint32_t nBaselines = makeAntennaVectors(in, beam, ant1, ant2);

    out.nBaselines() = nBaselines;
    out.antenna1() = ant1;
    out.antenna2() = ant2;

    // Make visibilities and flag vectors
    const vector<uint32_t>& beams = in.beam();
    const vector< complex<float> >& invis = in.visibilities();
    const vector<uint8_t>& inflag = in.flag();
    const uint32_t polidx = indexOfFirst(in.stokes(), pol);

    vector< complex<float> >& outvis = out.visibilities();
    outvis.clear();
    outvis.reserve(nBaselines * nChannels);

    vector<uint8_t>& outflag = out.flag();
    outflag.clear();
    outflag.reserve(nBaselines * nChannels);

    uint32_t baseline = 0;
    for (size_t row = 0; row < nRow; ++row) {
        if (beams[row] != beam) continue;
        for (size_t chan = 0; chan < nChannels; ++chan) {
            const size_t idx = in.index(row, chan, polidx);
            ASKAPDEBUGASSERT(idx < (nRow * nChannels * nPols));
            outvis.push_back(invis[idx]);
            outflag.push_back(inflag[idx]);
        }
        ++baseline;
    }

    ASKAPDEBUGASSERT(outvis.size() == nBaselines * nChannels);
    ASKAPDEBUGASSERT(outflag.size() == nBaselines * nChannels);

    return out;
}

uint32_t SubsetExtractor::makeAntennaVectors(const InputMessage& in, uint32_t beam,
        std::vector<uint32_t>& ant1out, std::vector<uint32_t>& ant2out)
{
    // Pre-conditions
    ASKAPCHECK(in.beam().size() == in.nRow(), "Beams vector incorrect size");
    ASKAPCHECK(in.antenna1().size() == in.nRow(), "Antenna 1 vector incorrect size");
    ASKAPCHECK(in.antenna2().size() == in.nRow(), "Antenna 2 vector incorrect size");

    ant1out.clear();
    ant2out.clear();
    const vector<uint32_t>& beams = in.beam();
    const vector<uint32_t>& ant1 = in.antenna1();
    const vector<uint32_t>& ant2 = in.antenna2();

    for (size_t i = 0; i < beams.size(); ++i) {
        if (beams[i] == beam) {
            ant1out.push_back(ant1[i]);
            ant2out.push_back(ant2[i]);
        }
    }

    // Post-conditions
    ASKAPCHECK(ant1out.size() == ant2out.size(), "Output vectors not of equal size");

    return ant1out.size();
}

size_t SubsetExtractor::indexOfFirst(const std::vector<uint32_t>& v, uint32_t val)
{
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == val) return i;
    }
    ASKAPTHROW(AskapError, "Value not found");
}
