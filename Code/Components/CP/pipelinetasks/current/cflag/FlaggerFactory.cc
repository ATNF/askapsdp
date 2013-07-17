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
#include "cflag/ElevationFlagger.h"
#include "cflag/StokesVFlagger.h"
#include "cflag/AmplitudeFlagger.h"

ASKAP_LOGGER(logger, ".FlaggerFactory");

using namespace std;
using namespace askap;
using namespace askap::cp::pipelinetasks;

void FlaggerFactory::appendFlaggers(vector< boost::shared_ptr<IFlagger> >& v1,
                                    const vector< boost::shared_ptr<IFlagger> > v2)
{
    v1.insert(v1.end(), v2.begin(), v2.end());
}

std::vector< boost::shared_ptr<IFlagger> > FlaggerFactory::build(
    const LOFAR::ParameterSet& parset, const casa::MeasurementSet& ms)
{
    vector< boost::shared_ptr<IFlagger> > flaggers;

    // Flaggers are responsible for inspecting the parset and instantiating
    // zero or more instances of themself as is required. New flaggers should
    // be added here:
    appendFlaggers(flaggers, SelectionFlagger::build(parset, ms));
    appendFlaggers(flaggers, ElevationFlagger::build(parset, ms));
    appendFlaggers(flaggers, StokesVFlagger::build(parset, ms));
    appendFlaggers(flaggers, AmplitudeFlagger::build(parset, ms));

    return flaggers;
}
