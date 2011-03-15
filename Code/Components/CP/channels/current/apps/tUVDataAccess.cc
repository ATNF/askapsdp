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
#include "uvchannel/uvdataaccess/UVChannelConstDataSource.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataIterator.h"
#include "uvchannel/uvdataaccess/UVChannelDataSelector.h"
#include "uvchannel/uvdataaccess/UVChannelDataConverter.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataAccessor.h"
#include "dataaccess/SharedIter.h"

ASKAP_LOGGER(logger, ".tUVDataAccess");

using namespace askap::accessors;
using namespace askap::cp::channels;

// main()
int main(int argc, char *argv[])
{
    ASKAPLOG_INIT("askap.log_cfg");

    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameter
    cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "tUVChannel.in");

    // Throw an exception if the parameter is not present
    parser.add(inputsPar, cmdlineparser::Parser::throw_exception);
    parser.process(argc, const_cast<char**> (argv));

    // Create a configuration parset
    LOFAR::ParameterSet parset(inputsPar);

    UVChannelConstDataSource ds(parset, "avg304");

    IDataSelectorPtr sel = ds.createSelector();
    sel->chooseChannels(1, 1);

    IDataConverterPtr conv = ds.createConverter();
    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
    conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));

    IConstDataSharedIter it = ds.createConstIterator(sel, conv);

    for (it.init(); it.hasMore(); it.next()) {
        ASKAPLOG_INFO_STR(logger, "Got an accessor for timestamp: " << it->time());
    }

    return 0;
}
