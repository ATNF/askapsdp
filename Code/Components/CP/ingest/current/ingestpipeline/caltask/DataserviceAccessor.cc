/// @file DataserviceAccessor.cc
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
#include "ingestpipeline/caltask/DataserviceAccessor.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <unistd.h>
#include <map>
#include <vector>

// ASKAPsoft includes
#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "casa/aipstype.h"
#include "casa/BasicSL/Complex.h"
#include "calibrationclient/CalibrationDataServiceClient.h"
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/JonesDTerm.h"
#include "calibrationclient/GenericSolution.h"

using namespace askap::cp::ingest;
using namespace askap::cp::caldataservice;

ASKAP_LOGGER(logger, ".DataserviceAccessor");

DataserviceAccessor::DataserviceAccessor(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& serviceName,
        const casa::Int updateInterval)
    : itsService(locatorHost, locatorPort, serviceName),
    itsUpdateInterval(updateInterval),
    itsStopRequested(false),
    itsGainID(-1), itsLeakageID(-1), itsBandpassID(-1)
{
    updateSolutions();
    const std::string msg = "solution available from calibration data service";
    if (itsGainSolution.get() == 0) {
        ASKAPLOG_DEBUG_STR(logger, "No gain " << msg);
    }
    if (itsLeakageSolution.get() == 0) {
        ASKAPLOG_DEBUG_STR(logger, "No leakage " << msg);
    }
    if (itsBandpassSolution.get() == 0) {
        ASKAPLOG_DEBUG_STR(logger, "No bandpass " << msg);
    }

    // Start the thread which updates solutions periodically
    itsUpdateThread = boost::shared_ptr<boost::thread>(
            new boost::thread(boost::bind(&DataserviceAccessor::updateThreadRun, this)));
}

DataserviceAccessor::~DataserviceAccessor()
{
    // Signal and wait for the update thread to stop
    itsStopRequested = true;
    if (itsUpdateThread.get()) {
        itsUpdateThread->join();
    }
    itsUpdateThread.reset();
}

casa::Complex DataserviceAccessor::getGain(casa::uInt ant, casa::uInt beam,
        ISolutionAccessor::Pol pol, casa::Bool& valid) const
{
    ASKAPCHECK(itsGainSolution.get(), "No gain solution available");
    boost::mutex::scoped_lock lock(itsGainMutex);

    // Locate the gain
    std::map<JonesIndex, JonesJTerm>::const_iterator pos = itsGainSolution->map().find(JonesIndex(ant, beam));
    if (pos == itsGainSolution->map().end()) {
        ASKAPLOG_DEBUG_STR(logger, "Gain not found for ant: " << ant << ", beam: " << beam);
        valid = false;
        return casa::Complex();
    }
    const JonesJTerm jterm = pos->second;
    if (pol == XX && jterm.g1IsValid()) {
        valid = true;
        return jterm.g1();
    } else if (pol == YY && jterm.g2IsValid()) {
        valid = true;
        return jterm.g2();
    } else {
        valid = false;
        return casa::Complex();
    }
}

casa::Complex DataserviceAccessor::getLeakage(casa::uInt ant, casa::uInt beam,
        ISolutionAccessor::LeakageTerm term, casa::Bool& valid) const
{
    ASKAPCHECK(itsLeakageSolution.get(), "No leakage solution available");
    boost::mutex::scoped_lock lock(itsLeakageMutex);

    // Locate the leakage
    std::map<JonesIndex, JonesDTerm>::const_iterator pos = itsLeakageSolution->map().find(JonesIndex(ant, beam));
    if (pos == itsLeakageSolution->map().end()) {
        valid = false;
        return casa::Complex();
    }
    const JonesDTerm dterm = pos->second;
    if (term == D12) {
        valid = true;
        return dterm.d12();
    } else if (term == D21) {
        valid = true;
        return dterm.d21();
    } else {
        ASKAPTHROW(AskapError, "Term type unknown/unhandled");
    }
}

casa::Complex DataserviceAccessor::getBandpass(casa::uInt ant, casa::uInt beam,
        casa::uInt chan, ISolutionAccessor::Pol pol,
        casa::Bool& valid) const
{
    ASKAPCHECK(itsBandpassSolution.get(), "No bandpass solution available");
    boost::mutex::scoped_lock lock(itsBandpassMutex);
    return casa::Complex();

    // Locate the bandpass
    std::map<JonesIndex, std::vector<JonesJTerm> >::const_iterator pos = itsBandpassSolution->map().find(JonesIndex(ant, beam));
    if (pos == itsBandpassSolution->map().end()) {
        valid = false;
        return casa::Complex();
    }
    const std::vector<JonesJTerm> jterms = pos->second;
    ASKAPCHECK(jterms.size() - 1 == chan, "channel index out of bounds");
    const JonesJTerm& jterm = jterms[chan];

    if (pol == XX && jterm.g1IsValid()) {
        valid = true;
        return jterm.g1();
    } else if (pol == YY && jterm.g2IsValid()) {
        valid = true;
        return jterm.g2();
    } else {
        valid = false;
        return casa::Complex();
    }
}

void DataserviceAccessor::updateSolutions(void)
{
    casa::Long newID;
    newID = itsService.getCurrentGainSolutionID();
    if (newID > itsGainID) {
        boost::shared_ptr<askap::cp::caldataservice::GainSolution> temp(new GainSolution(itsService.getGainSolution(newID)));
        ASKAPLOG_INFO_STR(logger, "Updating gain solution with ID: " << newID);
        boost::mutex::scoped_lock lock(itsGainMutex);
        itsGainSolution = temp;
        itsGainID = newID;
    }

    newID = itsService.getCurrentLeakageSolutionID();
    if (newID > itsLeakageID) {
        boost::shared_ptr<askap::cp::caldataservice::LeakageSolution> temp(new LeakageSolution(itsService.getLeakageSolution(newID)));
        ASKAPLOG_INFO_STR(logger, "Updating leakage solution with ID: " << newID);
        boost::mutex::scoped_lock lock(itsLeakageMutex);
        itsLeakageSolution = temp;
        itsLeakageID = newID;
    }

    newID = itsService.getCurrentBandpassSolutionID();
    if (newID > itsBandpassID) {
        boost::shared_ptr<askap::cp::caldataservice::BandpassSolution> temp(new BandpassSolution(itsService.getBandpassSolution(newID)));
        ASKAPLOG_INFO_STR(logger, "Updating bandpass solution with ID: " << newID);
        boost::mutex::scoped_lock lock(itsBandpassMutex);
        itsBandpassSolution = temp;
        itsBandpassID = newID;
    }
}

void DataserviceAccessor::updateThreadRun()
{
    while (!itsStopRequested) {
        updateSolutions();
        for (int i = 0; i < itsUpdateInterval; ++i) {
            if (itsStopRequested) {
                break;
            } else {
                sleep(1);
            }
        }
    }
}
