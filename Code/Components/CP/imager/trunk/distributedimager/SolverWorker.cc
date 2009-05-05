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

// Boost includes
#include <boost/scoped_ptr.hpp>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>
#include <casa/aipstype.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeCleaner.h>
#include <casa/Arrays/Array.h>

using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;

ASKAP_LOGGER(logger, ".SolverWorker");

SolverWorker::SolverWorker(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms,
        askap::scimath::Params::ShPtr model_p)
: m_parset(parset), m_comms(comms)
{
}

SolverWorker::~SolverWorker()
{
}

void SolverWorker::solveNE(askap::scimath::INormalEquations::ShPtr)
{
    const std::string solver_par = m_parset.getString("solver");
    const std::string algorithm_par = m_parset.getString("solver.Clean.algorithm", "MultiScale");
    const std::string distributed_par = m_parset.getString("solver.Clean.distributed", "False");

    // Workers do not participate in this operation, unless a distributed
    // clean is requested
    if (solver_par == "Clean" && algorithm_par == "MultiScale" &&
            distributed_par == "True") {
        distributedClean();
    }
}

void SolverWorker::writeModel(const std::string &postfix)
{
    // Workers do not participate in this operation
}

void SolverWorker::distributedClean(void)
{
    while (1) {
        // Ask the master for a workunit
        m_comms.sendString("next", cg_master);
        std::string ms = m_comms.receiveString(cg_master);
        if (ms != "ok") {
            // Indicates all workunits have been assigned already
            break;
        }

        int patchid;
        casa::Array<float> dirtyarray;
        casa::Array<float> psfarray;
        casa::Array<float> maskarray;
        casa::Array<float> modelarray;
        double _threshold;
        std::string thresholdUnits;
        double fractionalThreshold;
        std::vector<float> scales;
        int niter;
        double gain;

        m_comms.recvCleanRequest(patchid, dirtyarray, psfarray, maskarray, modelarray,
                _threshold, thresholdUnits, fractionalThreshold, scales,
                niter, gain);

        casa::ArrayLattice<float> dirty(dirtyarray);
        casa::ArrayLattice<float> psf(psfarray);
        boost::scoped_ptr< casa::ArrayLattice<float> > mask_p;
        boost::scoped_ptr< casa::ArrayLattice<float> > model_p;

        if (maskarray.size() > 0) {
            mask_p.reset(new casa::ArrayLattice<float>(maskarray));
        } else {
            ASKAPLOG_INFO_STR(logger, "Mask is empty");
        }

        if (modelarray.size() > 0) {
            model_p.reset(new casa::ArrayLattice<float>(modelarray));
        } else {
            // Create an empty model based on the shape of dirty.
            ASKAPLOG_INFO_STR(logger, "Model is empty");
            model_p.reset(new casa::ArrayLattice<float>(dirty.shape()));
        }

        // Create the Lattice Cleaner
        casa::LatticeCleaner<float> lc(psf, dirty);

        // Set the mask
        if (mask_p.get()) {
            lc.setMask(*mask_p, -1.0);
        }

        // Create a threshold
        casa::Quantity threshold(_threshold, thresholdUnits.c_str());

        // Setup LatticeCleaner
        lc.setscales(scales);
        lc.setcontrol(casa::CleanEnums::MULTISCALE,
                niter,
                gain,
                threshold,
                fractionalThreshold,
                false);
        lc.ignoreCenterBox(true);

        // Execute the clean
        lc.clean(*model_p);

        // Send the patch back to the master
        ASKAPLOG_INFO_STR(logger, "Sending CleanResponse for patchid " << patchid);
        m_comms.sendString("response", cg_master);
        m_comms.sendCleanResponse(patchid, model_p->asArray(), lc.strengthOptimum(), cg_master);
    }
    ASKAPLOG_INFO_STR(logger, "CleanWorker ACK no more work to do");
}
