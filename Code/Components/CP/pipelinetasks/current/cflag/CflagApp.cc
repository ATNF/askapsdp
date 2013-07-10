/// @file CflagApp.cc
///
/// @copyright (c) 2012 CSIRO
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
#include "cflag/CflagApp.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Common/ParameterSet.h"
#include "askap/StatReporter.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"

// Local package includes
#include "cflag/FlaggerFactory.h"
#include "cflag/IFlagger.h"
#include "cflag/FlaggingStats.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".CflagApp");

int CflagApp::run(int argc, char* argv[])
{
    StatReporter stats;
    const LOFAR::ParameterSet subset = config().makeSubset("Cflag.");

    // Open the measurement set
    const std::string dataset = subset.getString("dataset");
    casa::MeasurementSet ms(dataset, casa::Table::Update);

    // Create a vector of all the flagging strategies specified in the parset
    std::vector< boost::shared_ptr<IFlagger> > flaggers = FlaggerFactory::build(subset, ms);

    // Is this a dry run?
    const bool dryRun = subset.getBool("dryrun", false);
    if (dryRun) {
        ASKAPLOG_INFO_STR(logger, "!!!!! DRY RUN ONLY - MeasurementSet will not be updated !!!!!");
    }

    // Iterate over each row in the main table
    MSColumns msc(ms);
    const casa::uInt nRows = msc.nrow();
    std::vector< boost::shared_ptr<IFlagger> >::iterator it;
    unsigned long rowsAlreadyFlagged = 0;
    for (casa::uInt i = 0; i < nRows; ++i) {
        if (!msc.flagRow()(i)) {
            // Invoke each flagger for this row, but only while the row isn't flagged
            for (it = flaggers.begin(); it != flaggers.end(); ++it) {
                if (msc.flagRow()(i)) {
                    break;
                }
                (*it)->processRow(msc, i, dryRun);
            }
        } else {
            rowsAlreadyFlagged++;
        }
    }

    // Write out flagging statistics
    ASKAPLOG_INFO_STR(logger, "Summary:");
    ASKAPLOG_INFO_STR(logger, "  Rows already flagged: " << rowsAlreadyFlagged);
    for (it = flaggers.begin(); it != flaggers.end(); ++it) {
        const FlaggingStats stats = (*it)->stats();
        ASKAPLOG_INFO_STR(logger, "  " << stats.name
                              << " - Entire rows flagged: " << stats.rowsflagged
                              << ", Visibilities flagged: " << stats.visflagged);
    }

    stats.logSummary();
    return 0;
}
