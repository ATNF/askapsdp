/// @file SolverMaster.cc
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
#include "SolverMaster.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <fitting/Solver.h>
#include <fitting/Quality.h>
#include <measurementequation/ImageSolverFactory.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WienerPreconditioner.h>
#include <measurementequation/GaussianTaperPreconditioner.h>
#include <measurementequation/ImageMultiScaleSolver.h>
#include <casa/OS/Timer.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/common/DistributedImageSolverFactory.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".SolverMaster");

SolverMaster::SolverMaster(LOFAR::ParameterSet& parset,
        askap::cp::IBasicComms& comms,
        askap::scimath::Params::ShPtr model_p)
: itsSolverCore(parset, comms, model_p)
{
}

SolverMaster::~SolverMaster()
{
}

void SolverMaster::solveNE(askap::scimath::INormalEquations::ShPtr ne_p)
{
    itsSolverCore.solveNE(ne_p);
}

void SolverMaster::writeModel(const std::string& postfix)
{
    itsSolverCore.writeModel(postfix);
}
