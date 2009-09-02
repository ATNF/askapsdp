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

// Include own header file first
#include "ImageMultiScaleSolverWorker.h"

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

// Local includes
#include <distributedimager/common/IBasicComms.h>
#include <messages/IMessage.h>
#include <messages/CleanRequest.h>
#include <messages/CleanResponse.h>

using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;

ASKAP_LOGGER(logger, ".ImageMultiScaleSolverWorker");

ImageMultiScaleSolverWorker::ImageMultiScaleSolverWorker(
        const LOFAR::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
    : itsParset(parset), itsComms(comms) 
{
}

void ImageMultiScaleSolverWorker::solveNormalEquations(void)
{
    while (1) {
        // Ask the master for a workunit
        CleanResponse response;
        response.set_payloadType(CleanResponse::READY);
        itsComms.sendMessage(response, itsMaster);

        CleanRequest request;
        itsComms.receiveMessage(request, itsMaster);
        if (request.get_payloadType() == CleanRequest::FINALIZE) {
            // Indicates all workunits have been assigned already
            break;
        }

        int patchid = request.get_patchId();
        casa::Array<float>& dirtyarray  = request.get_dirty();
        casa::Array<float>& psfarray = request.get_psf();
        casa::Array<float>& maskarray = request.get_mask();
        casa::Array<float>& cleanarray = request.get_model();
        double _threshold = request.get_threshold();
        std::string thresholdUnits = request.get_thresholdUnits();
        double fractionalThreshold = request.get_fractionalThreshold();
        casa::Vector<float> scales = request.get_scales();
        int niter = request.get_niter();
        double gain = request.get_gain();

        casa::ArrayLattice<float> dirty(dirtyarray);
        casa::ArrayLattice<float> psf(psfarray);
        boost::scoped_ptr< casa::ArrayLattice<float> > mask_p;
        boost::scoped_ptr< casa::ArrayLattice<float> > clean_p;

        if (maskarray.size() > 0) {
            mask_p.reset(new casa::ArrayLattice<float>(maskarray));
        } else {
            ASKAPLOG_INFO_STR(logger, "Mask is empty");
        }

        if (cleanarray.size() > 0) {
            clean_p.reset(new casa::ArrayLattice<float>(cleanarray));
        } else {
            // Create an empty model based on the shape of dirty.
            ASKAPLOG_INFO_STR(logger, "Model is empty");
            clean_p.reset(new casa::ArrayLattice<float>(dirty.shape()));
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
        lc.clean(*clean_p);

        // Send the patch back to the master
        ASKAPLOG_INFO_STR(logger, "Sending CleanResponse for patchid " << patchid);
        response.set_payloadType(CleanResponse::RESULT);
        response.set_patchId(patchid);
        response.set_patch(clean_p->asArray());
        response.set_strengthOptimum(lc.strengthOptimum());
        itsComms.sendMessage(response, itsMaster);
    }
    ASKAPLOG_INFO_STR(logger, "CleanWorker ACK no more work to do");
};

