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
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <measurementequation/SynthesisParamsHelper.h>


using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace LOFAR::ACC::APS;

ASKAP_LOGGER(logger, ".PreDifferMaster");

PreDifferMaster::PreDifferMaster(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms) : m_parset(parset), m_comms(comms)
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
    m_comms.broadcastModel(model_p);

    // Send work orders to the worker processes
    std::vector<std::string> ms = getDatasets(m_parset);
    if (ms.size() == 0) {
        ASKAPTHROW (std::runtime_error, "No datasets specified in the parameter set file");
    }

    // Wait for all work units to complete processing, handling out
    // more work to the workers as needed
    for (unsigned int n = 0; n < ms.size(); ++n) {
        // TODO: Waiting for a string is a dumb way for the worker to indicate
        // it wants more work to do. Need a MUCH better way of doing this. Some
        // sort of command message incorporating this plus the "no more workunits"
        // message (below) could be developed.
        int source;
        m_comms.receiveStringAny(source);

        ASKAPLOG_INFO_STR(logger, "Master is allocating workunit " << ms[n]
                << " to worker " << source);
        m_comms.sendString(ms[n], source);
    }

    // Send each process an empty string to indicate
    // there are no more workunits on offer (TODO: Need to find
    // a better way of doing this)
    for (int dest = 1; dest < m_comms.getNumNodes(); ++dest) {
        m_comms.receiveString(dest);
        m_comms.sendString("", dest);
    }

    // Finally, wait for the workers/accumulators to send all the normal
    // equations to the master. The count argument tracks how many datasets
    // were processed to arrive at the normal equation object. The master
    // does not proceed until the results for all datasets have been
    // accounted for.
    unsigned int count = 0;
    int responsible = m_comms.responsible();
    for (int i = 0; i < responsible; ++i) {
        int source;
        int recvcount = 0;
        askap::scimath::INormalEquations::ShPtr recv_ne_p = m_comms.receiveNE(source, recvcount);
        count += recvcount;

        // Merge the received normal equations
        if (recvcount > 0) {
            ne_p->merge(*recv_ne_p);
        }
        recv_ne_p.reset();

        ASKAPLOG_INFO_STR(logger, "Received " << recvcount << " normal equations from worker " 
                << source << ". Still waiting for " << ms.size() - count << ".");
    }

    ASKAPCHECK(count == ms.size(), "Results for one or more datasets missing");

    return ne_p;
}

/// Utility function to get dataset names from parset.
std::vector<std::string> PreDifferMaster::getDatasets(ParameterSet& parset)
{
    if (parset.isDefined("dataset") && parset.isDefined("dataset0")) {
        ASKAPTHROW (std::runtime_error, "Both dataset and dataset0 are specified in the parset");
    }

    // First look for "dataset" and if that does not exist try "dataset0"
    std::vector<std::string> ms;
    if (parset.isDefined("dataset")) {
        ms = m_parset.getStringVector("dataset");
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
