/// @file PreDifferMaster.cc
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
#include "PreDifferMaster.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <measurementequation/SynthesisParamsHelper.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/continuum/ReductionLogic.h"
#include "messages/UpdateModel.h"
#include "messages/PreDifferRequest.h"
#include "messages/PreDifferResponse.h"

using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".PreDifferMaster");

PreDifferMaster::PreDifferMaster(LOFAR::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
: itsParset(parset), itsComms(comms)
{
}

PreDifferMaster::~PreDifferMaster()
{
}

askap::scimath::INormalEquations::ShPtr PreDifferMaster::calcNE(askap::scimath::Params::ShPtr model_p)
{
    // This Normal Equations object will combine all the results
    // from the worker processes.
    askap::scimath::INormalEquations::ShPtr ne_p =
        ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*model_p));

    // Broadcast the model to the workers
    UpdateModel message;
    message.set_model(model_p);
    itsComms.sendMessageBroadcast(message);

    // Read from the configruation the list of datasets to process
    std::vector<std::string> ms = getDatasets(itsParset);
    if (ms.size() == 0) {
        ASKAPTHROW (std::runtime_error, "No datasets specified in the parameter set file");
    }

    // Send work orders to the worker processes, handling out
    // more work to the workers as needed.
    for (unsigned int n = 0; n < ms.size(); ++n) {
        int id; // Id of the process the message is received from

        // Rather than just sending work to all workers, the workers actually
        // first send a request for work.
        PreDifferResponse response;
        itsComms.receiveMessageAnySrc(response, id);
        ASKAPCHECK(response.get_payloadType() == PreDifferResponse::READY,
                "Expected only READY payloads at this time");

        ASKAPLOG_INFO_STR(logger, "Master is allocating workunit " << ms[n]
                << " to worker " << id);
        PreDifferRequest request;
        request.set_payloadType(PreDifferRequest::WORK);
        request.set_dataset(ms[n]);
        itsComms.sendMessage(request, id);
    }

    // Send each process an empty string to indicate
    // there are no more workunits on offer.
    for (int id = 1; id < itsComms.getNumNodes(); ++id) {
        // First get the request for more work message from the worker
        PreDifferResponse response;
        itsComms.receiveMessage(response, id);
        ASKAPCHECK(response.get_payloadType() == PreDifferResponse::READY,
                "Expected only READY payloads at this time");

        // Now send the finalize command
        PreDifferRequest request;
        request.set_payloadType(PreDifferRequest::FINALIZE);
        itsComms.sendMessage(request, id);
    }

    // Finally, wait for the workers/accumulators to send all the normal
    // equations to the master. The count argument tracks how many datasets
    // were processed to arrive at the normal equation object. The master
    // does not proceed until the results for all datasets have been
    // accounted for.
    ReductionLogic rlogic(itsComms.getId(), itsComms.getNumNodes());
    unsigned int count = 0;
    int responsible = rlogic.responsible();
    for (int i = 0; i < responsible; ++i) {
        int id; // Id of the process the message is received from
        PreDifferResponse response;
        itsComms.receiveMessageAnySrc(response, id);

        ASKAPCHECK(response.get_payloadType() == PreDifferResponse::RESULT,
                "Expected only RESULT payloads at this time");

        // Merge the received normal equations
        int recvcount = response.get_count();
        if (recvcount > 0) {
            ne_p->merge(*response.get_normalEquations());
            count += recvcount;
        }

        ASKAPLOG_INFO_STR(logger, "Received " << recvcount << " normal equations from worker " 
                << id << ". Still waiting for " << ms.size() - count << ".");
    }

    ASKAPCHECK(count == ms.size(), "Results for one or more datasets missing");

    return ne_p;
}

/// Utility function to get dataset names from parset.
std::vector<std::string> PreDifferMaster::getDatasets(LOFAR::ParameterSet& parset)
{
    if (parset.isDefined("dataset") && parset.isDefined("dataset0")) {
        ASKAPTHROW (std::runtime_error, "Both dataset and dataset0 are specified in the parset");
    }

    // First look for "dataset" and if that does not exist try "dataset0"
    std::vector<std::string> ms;
    if (parset.isDefined("dataset")) {
        ms = itsParset.getStringVector("dataset");
    } else {
        std::string key = "dataset0";   // First key to look for
        long idx = 0;
        while (parset.isDefined(key)) {
            std::string value = parset.getString(key);
            ms.push_back(value);

            std::ostringstream ss;
            ss << "dataset" << idx+1;
            key = ss.str();
            ++idx;
        }
    }

    return ms;
}
