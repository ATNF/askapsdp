/// @file SolverWorker.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "SolverWorker.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>

using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;

ASKAP_LOGGER(logger, ".SolverWorker");

SolverWorker::SolverWorker(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IImagerComms& comms,
        askap::scimath::Params::ShPtr model_p)
{
    // Workers do not participate in this operation
}

SolverWorker::~SolverWorker()
{
    // Workers do not participate in this operation
}

void SolverWorker::solveNE(askap::scimath::INormalEquations::ShPtr)
{
    // Workers do not participate in this operation
}

void SolverWorker::writeModel(const std::string &postfix)
{
    // Workers do not participate in this operation
}
