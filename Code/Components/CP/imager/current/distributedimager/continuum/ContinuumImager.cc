/// @file ContinuumImager.cc
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
#include "ContinuumImager.h"

// Include package level header file
#include <askap_imager.h>

// System includes
#include <string>

// Boost includes
#include <boost/scoped_ptr.hpp>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <measurementequation/SynthesisParamsHelper.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/continuum/IPreDifferTask.h"
#include "distributedimager/continuum/PreDifferMaster.h"
#include "distributedimager/continuum/PreDifferWorker.h"
#include "distributedimager/continuum/ISolverTask.h"
#include "distributedimager/continuum/SolverMaster.h"
#include "distributedimager/continuum/SolverWorker.h"

ASKAP_LOGGER(logger, ".ContinuumImager");

using namespace askap::cp;
using namespace askap;
using namespace askap::synthesis;
using namespace askap::scimath;

ContinuumImager::ContinuumImager(LOFAR::ParameterSet& parset,
        askap::cp::MPIBasicComms& comms) : itsParset(parset), itsComms(comms)
{
    if (isMaster()) {
        ASKAPLOG_INFO_STR(logger, "ASKAP Distributed Continuum Imager - " << ASKAP_PACKAGE_VERSION);
    }
}

ContinuumImager::~ContinuumImager()
{
}

void ContinuumImager::run(void)
{
    // Setup the model (master only)
    if (isMaster()) {
        itsModel.reset(new Params());
        // Set up image handler
        SynthesisParamsHelper::setUpImageHandler(itsParset);

        bool reuseModel = itsParset.getBool("Images.reuse", false);
        if (reuseModel) {
            ASKAPLOG_INFO_STR(logger, "Reusing model images stored on disk");
            SynthesisParamsHelper::loadImages(itsModel, itsParset.makeSubset("Images."));
        } else {
            ASKAPLOG_INFO_STR(logger, "Initializing the model images");

            /// Create the specified images from the definition in the
            /// parameter set. We can solve for any number of images
            /// at once (but you may/will run out of memory!)
            SynthesisParamsHelper::setUpImages(itsModel, itsParset.makeSubset("Images."));
        }
    }

    double targetPeakResidual = SynthesisParamsHelper::convertQuantity(
            itsParset.getString("threshold.majorcycle","-1Jy"),"Jy");
    const bool writeAtMajorCycle = itsParset.getBool("Images.writeAtMajorCycle",false);
    int nCycles = itsParset.getInt32("ncycles", 0);

    boost::scoped_ptr<IPreDifferTask> prediffer_p;
    boost::scoped_ptr<ISolverTask> solver_p;
    if (isMaster()) {
        prediffer_p.reset(new PreDifferMaster(itsParset, itsComms));
        solver_p.reset(new SolverMaster(itsParset, itsComms, itsModel));
    } else {
        prediffer_p.reset(new PreDifferWorker(itsParset, itsComms));
        solver_p.reset(new SolverWorker(itsParset, itsComms, itsModel));
    }

    if (nCycles == 0) {
        // No cycling - just make a dirty image
        askap::scimath::INormalEquations::ShPtr ne_p = prediffer_p->calcNE(itsModel);
        solver_p->solveNE(ne_p);
    } else {
        // Perform multiple major cycles
        for (int cycle = 0; cycle < nCycles; ++cycle) {
            if (isMaster()) {
                ASKAPLOG_INFO_STR(logger, "*** Starting major cycle " << cycle << " ***" );
            }
            askap::scimath::INormalEquations::ShPtr ne_p = prediffer_p->calcNE(itsModel);
            solver_p->solveNE(ne_p);

            if (isMaster()) {

                if (itsModel->has("peak_residual")) {
                    const double peak_residual = itsModel->scalarValue("peak_residual");
                    ASKAPLOG_INFO_STR(logger, "Reached peak residual of " << peak_residual);
                    if (peak_residual < targetPeakResidual) {
                        ASKAPLOG_INFO_STR(logger, "It is below the major cycle threshold of "
                                << targetPeakResidual << " Jy. Stopping.");
                        break;
                    } else {
                        if (targetPeakResidual < 0) {
                            ASKAPLOG_INFO_STR(logger, "Major cycle flux threshold is not used.");
                        } else {
                            ASKAPLOG_INFO_STR(logger, "It is above the major cycle threshold of "
                                    << targetPeakResidual << " Jy. Continuing.");
                        }
                    }
                }
                if (cycle+1 >= nCycles) {
                    ASKAPLOG_INFO_STR(logger, "Reached " << nCycles << " cycle(s), the maximum number of major cycles. Stopping.");
                }

                if (writeAtMajorCycle) {
                solver_p->writeModel(std::string(".majorcycle.")+utility::toString(cycle+1));
                }
            }
        }
        if (isMaster()) {
            ASKAPLOG_INFO_STR(logger, "*** Finished major cycles ***" );
        }
        askap::scimath::INormalEquations::ShPtr ne_p = prediffer_p->calcNE(itsModel);
    }

    solver_p->writeModel("");
}

bool ContinuumImager::isMaster(void)
{
    return (itsComms.getId() == itsMaster) ? true : false;
}
