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
}

void CalibrationDataServiceClient::addLeakageSolution(const LeakageSolution& sol)
{
}

void CalibrationDataServiceClient::addBandpassSolution(const BandpassSolution& sol)
{
}
