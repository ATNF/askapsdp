/// @file CalibrationDataServiceClient.cc
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
#include "CalibrationDataServiceClient.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "askap/AskapError.h"
#include "Ice/Ice.h"
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"
#include "CalibrationDataService.h" // Ice generated interface

// Local package includes
#include "calibrationclient/GainSolution.h"
#include "calibrationclient/LeakageSolution.h"
#include "calibrationclient/BandpassSolution.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::caldataservice;

CalibrationDataServiceClient::CalibrationDataServiceClient(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& serviceName)
{
    askap::cp::icewrapper::CommunicatorConfig config(locatorHost, locatorPort);
    config.setProperty("Ice.MessageSizeMax", "131072");
    askap::cp::icewrapper::CommunicatorFactory commFactory;
    itsComm = commFactory.createCommunicator(config);

    ASKAPDEBUGASSERT(itsComm);

    Ice::ObjectPrx base = itsComm->stringToProxy(serviceName);
    itsService = askap::interfaces::caldataservice::ICalibrationDataServicePrx::checkedCast(base);

    if (!itsService) {
        ASKAPTHROW(AskapError, "CalibrationDataService proxy is invalid");
    }
}

CalibrationDataServiceClient::~CalibrationDataServiceClient()
{
    itsComm->destroy();
}

void CalibrationDataServiceClient::addGainSolution(const GainSolution& sol)
{
    // Pre-conditions
    ASKAPCHECK(sol.nAntenna() == static_cast<casa::Short>(sol.antennaIndex().size()), 
            "Antenna index length != nAntenna");
    ASKAPCHECK(sol.nBeam() == static_cast<casa::Short>(sol.beamIndex().size()), 
            "Beam index length != nBeam");

    // Convert the casa type GainSolution to the ice GainSolution
    askap::interfaces::calparams::TimeTaggedGainSolution ice_sol;
    ice_sol.timestamp = sol.timestamp();

    for (casa::Short ant = 0; ant < sol.nAntenna(); ++ant) {
        for (casa::Short beam = 0; beam < sol.nBeam(); ++beam) {
            askap::interfaces::calparams::JonesIndex jindex;
            jindex.antennaID = sol.antennaIndex()(ant);
            jindex.beamID = sol.beamIndex()(beam);
            ice_sol.gain[jindex] = toIce(sol.gains()(ant,beam));
        }
    }

    itsService->addGainsSolution(ice_sol);

    // Post-conditions
    ASKAPCHECK(static_cast<casa::Int>(ice_sol.gain.size()) == sol.nAntenna() * sol.nBeam(),
            "Map size incorrect - missing elements");
}

void CalibrationDataServiceClient::addLeakageSolution(const LeakageSolution& sol)
{
    // Pre-conditions
    ASKAPCHECK(sol.nAntenna() == static_cast<casa::Short>(sol.antennaIndex().size()), 
            "Antenna index length != nAntenna");
    ASKAPCHECK(sol.nBeam() == static_cast<casa::Short>(sol.beamIndex().size()), 
            "Beam index length != nBeam");

    // Convert the casa type LeakageSolution to the ice LeakageSolution
    askap::interfaces::calparams::TimeTaggedLeakageSolution ice_sol;
    ice_sol.timestamp = sol.timestamp();

    for (casa::Short ant = 0; ant < sol.nAntenna(); ++ant) {
        for (casa::Short beam = 0; beam < sol.nBeam(); ++beam) {
            askap::interfaces::calparams::JonesIndex jindex;
            jindex.antennaID = sol.antennaIndex()(ant);
            jindex.beamID = sol.beamIndex()(beam);
            askap::interfaces::DoubleComplex leakage;
            leakage.real = sol.leakage()(ant, beam).real();
            leakage.imag = sol.leakage()(ant, beam).imag();
            ice_sol.leakage[jindex] = leakage;
        }
    }

    itsService->addLeakageSolution(ice_sol);

    // Post-conditions
    ASKAPCHECK(static_cast<casa::Int>(ice_sol.leakage.size()) == sol.nAntenna() * sol.nBeam(),
            "Map size incorrect - missing elements");
}

void CalibrationDataServiceClient::addBandpassSolution(const BandpassSolution& sol)
{
    // Pre-conditions
    ASKAPCHECK(sol.nAntenna() == static_cast<casa::Short>(sol.antennaIndex().size()), 
            "Antenna index length != nAntenna");
    ASKAPCHECK(sol.nBeam() == static_cast<casa::Short>(sol.beamIndex().size()), 
            "Beam index length != nBeam");
    ASKAPCHECK(sol.nChan() == static_cast<casa::Int>(sol.chanIndex().size()), 
            "Channel index length != nChan");

    // Convert the casa type BandpassSolution to the ice BandpassSolution
    askap::interfaces::calparams::TimeTaggedBandpassSolution ice_sol;
    ice_sol.timestamp = sol.timestamp();

    for (casa::Short ant = 0; ant < sol.nAntenna(); ++ant) {
        for (casa::Short beam = 0; beam < sol.nBeam(); ++beam) {
            askap::interfaces::calparams::JonesIndex jindex;
            jindex.antennaID = sol.antennaIndex()(ant);
            jindex.beamID = sol.beamIndex()(beam);

            askap::interfaces::calparams::FrequencyDependentJTerm fdjterm;
            for (casa::Int chan = 0; chan < sol.nChan(); ++chan)
            {
                fdjterm.bandpass.push_back(toIce(sol.bandpass()(ant,beam, chan)));
            }
            ice_sol.bandpass[jindex] = fdjterm;
        }
    }

    itsService->addBandpassSolution(ice_sol);

    // Post-conditions
    ASKAPCHECK(static_cast<casa::Int>(ice_sol.bandpass.size()) == sol.nAntenna() * sol.nBeam(),
            "Map size incorrect - missing elements");
}

askap::interfaces::calparams::JonesJTerm CalibrationDataServiceClient::toIce(askap::cp::caldataservice::JonesJTerm jterm)
{
    askap::interfaces::calparams::JonesJTerm ice_jterm;
    ice_jterm.g1.real = jterm.g1().real();
    ice_jterm.g1.imag = jterm.g1().imag();
    ice_jterm.g1Valid = jterm.g1IsValid();
    ice_jterm.g2.real = jterm.g2().real();
    ice_jterm.g2.imag = jterm.g2().imag();
    ice_jterm.g2Valid = jterm.g2IsValid();
    return ice_jterm;
}
