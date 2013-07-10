/// @file FlaggerFactory.cc
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
#include "FlaggerFactory.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <sstream>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "boost/shared_ptr.hpp"
#include "ms/MeasurementSets/MeasurementSet.h"

// Local package includes
#include "cflag/IFlagger.h"
#include "cflag/SelectionFlagger.h"
#include "cflag/StokesVFlagger.h"
#include "cflag/ElevationFlagger.h"

ASKAP_LOGGER(logger, ".FlaggerFactory");

using namespace std;
using namespace askap;
using namespace askap::cp::pipelinetasks;

std::vector< boost::shared_ptr<IFlagger> > FlaggerFactory::build(
    const LOFAR::ParameterSet& parset, const casa::MeasurementSet& ms)
{
    vector< boost::shared_ptr<IFlagger> > flaggers;

    // Create Selection flaggers
    string key = "selection_flagger.rules";
    if (parset.isDefined(key)) {
        const vector<string> rules = parset.getStringVector(key);
        vector<string>::const_iterator it;
        for (it = rules.begin(); it != rules.end(); ++it) {
            ASKAPLOG_DEBUG_STR(logger, "Processing rule: " << *it);
            stringstream ss;
            ss << "selection_flagger." << *it << ".";
            const LOFAR::ParameterSet subset = parset.makeSubset(ss.str());
            flaggers.push_back(boost::shared_ptr<IFlagger>(new SelectionFlagger(subset, ms)));
        }
    }

    // Create Stokes V flagger
    key = "stokesv_flagger.enable";
    if (parset.isDefined(key) && parset.getBool(key)) {
        const LOFAR::ParameterSet subset = parset.makeSubset("stokesv_flagger.");
        flaggers.push_back(boost::shared_ptr<IFlagger>(new StokesVFlagger(subset, ms)));
    }

    // Create elevation based flagger
    key = "elevation_flagger.enable";
    if (parset.isDefined(key) && parset.getBool(key)) {
        const LOFAR::ParameterSet subset = parset.makeSubset("elevation_flagger.");
        flaggers.push_back(boost::shared_ptr<IFlagger>(new ElevationFlagger(subset, ms)));
    }

    return flaggers;
}
