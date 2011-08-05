/// @file SimPlayback.cc
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
#include "SimPlayback.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>
#include <sstream>
#include <vector>
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "boost/shared_ptr.hpp"
#include "casa/OS/Timer.h"

// Local package includes
#include "simplayback/ISimulator.h"
#include "simplayback/CorrelatorSimulator.h"
#include "simplayback/TosSimulator.h"

// Using
using namespace askap::cp;

ASKAP_LOGGER(logger, ".SimPlayback");

SimPlayback::SimPlayback(const LOFAR::ParameterSet& parset)
    : itsParset(parset.makeSubset("playback."))
{
    MPI_Comm_rank(MPI_COMM_WORLD, &itsRank);
    MPI_Comm_size(MPI_COMM_WORLD, &itsNumProcs);
    if (itsRank == 0) {
        validateConfig();
    }
}

SimPlayback::~SimPlayback()
{
}

void SimPlayback::validateConfig(void)
{
    if (itsRank == 0) {
        const std::string nShelvesKey = "corrsim.n_shelves";

        const int nShelves = itsParset.getInt32(nShelvesKey);
        ASKAPCHECK(itsNumProcs == (nShelves+1),
                "Incorrect number of ranks for the requested configration");

        // Build a list of required keys    
        std::vector<std::string> requiredKeys;
        requiredKeys.push_back(nShelvesKey);
        requiredKeys.push_back("tossim.ice.locator_host");
        requiredKeys.push_back("tossim.ice.locator_port");
        requiredKeys.push_back("tossim.icestorm.topicmanager");
        requiredKeys.push_back("tossim.icestorm.topic");
        for (int i = 0; i < nShelves; ++i) {
            std::ostringstream ss;
            ss << "corrsim.shelf" << i+1 << ".";

            std::string dataset = ss.str();
            dataset.append("dataset");
            requiredKeys.push_back(dataset);

            std::string hostname = ss.str();
            hostname.append("out.hostname");
            requiredKeys.push_back(hostname);

            std::string port = ss.str();
            port.append("out.port");
            requiredKeys.push_back(port);
        }

        // Now check the required keys are present
        std::vector<std::string>::const_iterator it;
        it = requiredKeys.begin();

        while (it != requiredKeys.end()) {
            if (!itsParset.isDefined(*it)) {
                ASKAPTHROW(AskapError, "Required key not present in parset: " << *it);
            }
            ++it;
        }
    }
}

boost::shared_ptr<TosSimulator> SimPlayback::makeTosSim(void)
{
    const std::string filename = itsParset.getString("corrsim.shelf1.dataset");
    const std::string locatorHost = itsParset.getString("tossim.ice.locator_host");
    const std::string locatorPort = itsParset.getString("tossim.ice.locator_port");
    const std::string topicManager = itsParset.getString("tossim.icestorm.topicmanager");
    const std::string topic = itsParset.getString("tossim.icestorm.topic");

    return boost::shared_ptr<TosSimulator>(new TosSimulator(filename,
                locatorHost, locatorPort, topicManager, topic));
}

boost::shared_ptr<CorrelatorSimulator> SimPlayback::makeCorrelatorSim(void)
{
    std::ostringstream ss;
    ss << "corrsim.shelf" << itsRank << ".";
    const LOFAR::ParameterSet subset = itsParset.makeSubset(ss.str());
    const std::string dataset = subset.getString("dataset");
    const std::string hostname = subset.getString("out.hostname");
    const std::string port = subset.getString("out.port");
    const unsigned int expansion = itsParset.getUint32("corrsim.expansion_factor", 1);
    return boost::shared_ptr<CorrelatorSimulator>(
            new CorrelatorSimulator(dataset, hostname, port, expansion));
}

void SimPlayback::run(void)
{
    // Wait for all processes to get here. The master alone checks the config
    // file so this barrier ensures the configuration has been validated
    // before all processes go and use it. If the master finds a problem
    // an MPI_Abort is called.
    MPI_Barrier(MPI_COMM_WORLD);

    boost::shared_ptr<ISimulator> sim;
    if (itsRank == 0) {
        sim = makeTosSim();
    } else {
        sim = makeCorrelatorSim();
    }

    // In units of micro-second the "period" constant is the integration
    // time
    const unsigned long periodPar = itsParset.getUint32("period", 5);
    const unsigned long period = periodPar * 1000L * 1000L;

    // Simulate until the simulators advise there is no longer any data
    bool moreData = true;
    while (moreData) {
        const unsigned long nextTime =
            static_cast<unsigned long>(MPI_Wtime() * 1000.0 * 1000.0) + period;
        MPI_Barrier(MPI_COMM_WORLD);
        moreData = sim->sendNext();

        // Wait before sending the next integration
        unsigned long now = static_cast<unsigned long>(MPI_Wtime() * 1000.0 * 1000.0);

        // But first check and report if we are behind schedule
        if (itsRank == 0) {
            if (now > nextTime) {
                ASKAPLOG_DEBUG_STR(logger, "Running slower than integration cycle period");
            }
        }

        while (now < nextTime) {
            const unsigned long sleepTime = nextTime - now;
            usleep(sleepTime);
            now = static_cast<unsigned long>(MPI_Wtime() * 1000.0 * 1000.0);
        }
    }
}
