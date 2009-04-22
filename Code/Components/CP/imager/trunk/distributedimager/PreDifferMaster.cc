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
        askap::cp::IImagerComms& comms) : m_parset(parset), m_comms(comms)
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

    // Send the work to the workers
    for (unsigned int n = 0; n < ms.size(); ++n) {
        // Initially send one workunit to each worker,
        // from ID 1 to (getNumNodes() - 1)
        for (int dest = 1; dest < m_comms.getNumNodes() && n < ms.size(); ++dest) {
            ASKAPLOG_INFO_STR(logger, "Master is allocating workunit " << ms[n]
                    << " to worker " << dest);
            m_comms.sendString(ms[n], dest);
            ++n;
        }

        // Wait for all work units to complete processing, handling out
        // more work to the workers as needed
        for (unsigned int completed = 0; completed < ms.size(); ++completed) {
            int source;
            askap::scimath::INormalEquations::ShPtr recv_ne_p = m_comms.receiveNE(source);

            // If there is still work to be distributed, send this worker a new
            // work unit
            if (n < ms.size()) {
                ASKAPLOG_INFO_STR(logger, "Master is allocating workunit " << ms[n]
                        << " to worker " << source);
                m_comms.sendString(ms[n], source);
                ++n;
            }

            // Merge the received normal equations
            ASKAPLOG_INFO_STR(logger, "Merging normal equations from worker " << source);
            ne_p->merge(*recv_ne_p);
        }

        // Finally, send each process an empty string to indicate
        // there are no more workunits on offer (TODO: Need to find
        // a better way of doing this)
        for (int dest = 1; dest < m_comms.getNumNodes(); ++dest) {
            m_comms.sendString("", dest);
        }

    }

    return ne_p;
}

/// Utility function to get dataset names from parset.
std::vector<std::string> PreDifferMaster::getDatasets(ParameterSet& parset)
{
    if(parset.isDefined("dataset") && parset.isDefined("dataset0")) {
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
