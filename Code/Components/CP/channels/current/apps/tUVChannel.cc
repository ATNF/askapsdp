/// @file tUVChannel.cc
///
/// @description
/// This program executes a very simple testcase for the central processor
/// uv-channel.
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

// System includes
#include <iostream>
#include <string>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "uvchannel/UVChannelPublisher.h"

using namespace askap::cp::channels;

// main()
int main(int argc, char *argv[])
{
    const unsigned int nMessages = 5;
    const unsigned int nChans = 152;

    // Create a configuration parset
    LOFAR::ParameterSet parset;
    parset.add("uvchannel.brokers", "[broker1]");
    parset.add("uvchannel.broker.broker1.host", "localhost");
    parset.add("uvchannel.broker.broker1.port", "61616");
    parset.add("uvchannel.channels", "[avg304]");

    parset.add("uvchannel.channel.avg304.topic_prefix", "avg304");
    parset.add("uvchannel.channel.avg304.nblocks", "1");
    parset.add("uvchannel.channel.avg304.block_1", "[1, 304, broker1]");

    // Setup the channel
    UVChannelPublisher pub(parset, "avg304");

    // Create a VisChunk
    // This is the size of a BETA VisChunk, 21 baselines (including
    // auto correlations) * 36 beams (maximum number of beams)
    const unsigned int nRows = 21 * 36;
    const unsigned int nChansPerChunk = 1;
    const unsigned int nPols = 4;
    askap::cp::common::VisChunk data(nRows, nChansPerChunk, nPols);

    for (unsigned int i = 1; i <= nMessages; ++i) {
        for (unsigned int c = 1; c <= nChans; ++c) {
            std::cout << "Iteration " << i << " channel " << c << std::endl;
            pub.publish(data, c);
        }
    }

    return 0;
}
