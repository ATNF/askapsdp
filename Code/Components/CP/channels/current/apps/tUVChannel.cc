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

// Package level header file
#include "askap_channels.h"

// System includes
#include <iostream>
#include <string>
#include <unistd.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "uvchannel/UVChannelPublisher.h"
#include "uvchannel/UVChannelConsumer.h"
#include "uvchannel/IUVChannelListener.h"

ASKAP_LOGGER(logger, ".tUVChannel");

using namespace askap::cp::channels;

class MyListener : public IUVChannelListener {
    public:
        MyListener() : itsCount(0) { }

        long getCount(void) { return itsCount; }

    protected:
        virtual void onMessage(const boost::shared_ptr<askap::cp::common::VisChunk> message) {
            itsCount++;
        }

    private:
        long itsCount;
};

// main()
int main(int argc, char *argv[])
{
    ASKAPLOG_INIT("askap.log_cfg");

    const unsigned int nMessages = 6;
    const unsigned int nChans = 304;

    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameter
    cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "tUVChannel.in");

    // Throw an exception if the parameter is not present
    parser.add(inputsPar, cmdlineparser::Parser::throw_exception);
    parser.process(argc, const_cast<char**> (argv));

    // Create a configuration parset
    LOFAR::ParameterSet parset(inputsPar);

    // Setup the publisher
    UVChannelPublisher pub(parset, "avg304");

    // Setup the consumer
    MyListener listener;
    UVChannelConsumer consumer(parset, "avg304", &listener);

    for (unsigned int c = 1; c <= nChans; ++c) {
        consumer.addSubscription(c);
    }

    // Create a VisChunk
    // This is the size of a BETA VisChunk, 21 baselines (including
    // auto correlations) * 36 beams (maximum number of beams)
    const unsigned int nRows = 21 * 36;
    const unsigned int nChansPerChunk = 1;
    const unsigned int nPols = 4;
    askap::cp::common::VisChunk data(nRows, nChansPerChunk, nPols);

    for (unsigned int i = 1; i <= nMessages; ++i) {
        for (unsigned int c = 1; c <= nChans; ++c) {
            ASKAPLOG_INFO_STR(logger, "Iteration " << i << " channel " << c);
            pub.publish(data, c);

            // Don't let the publisher get too far ahead of the consumer
            if ((i * nChans) > listener.getCount()) {
                usleep(5000);
            }
        }
    }

    ASKAPLOG_INFO_STR(logger, "Waiting for messages to arrive...");
    const int expected = nMessages * nChans;

    for (int i = 0; i < 5; ++i) {
        if (listener.getCount() == expected) break;

        sleep(1);
    }

    ASKAPLOG_INFO_STR(logger, "Got " << listener.getCount() << ", expected " << expected);

    return (listener.getCount() == expected);
}
