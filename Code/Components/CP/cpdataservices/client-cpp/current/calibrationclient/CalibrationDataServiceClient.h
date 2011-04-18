/// @file CalibrationDataServiceClient.h
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

#ifndef ASKAP_CP_CPDATASERVICES_CALIBRATIONDATASERVICE_H
#define ASKAP_CP_CPDATASERVICES_CALIBRATIONDATASERVICE_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "CalibrationDataService.h" // Ice generated interface

// Local package includes
#include "calibrationclient/GainSolution.h"
#include "calibrationclient/LeakageSolution.h"
#include "calibrationclient/BandpassSolution.h"

namespace askap {
namespace cp {
namespace caldataservice {

class CalibrationDataServiceClient {

    public:
        /// Constructor
        CalibrationDataServiceClient(const std::string& locatorHost,
                                     const std::string& locatorPort,
                                     const std::string& serviceName = "CalibrationDataService");

        /// Destructor.
        ~CalibrationDataServiceClient();

        void addGainSolution(const GainSolution& sol);

        void addLeakageSolution(const LeakageSolution& sol);

        void addBandpassSolution(const BandpassSolution& sol);
    private:

        askap::interfaces::calparams::JonesJTerm toIce(askap::cp::caldataservice::JonesJTerm jterm);

        // Ice Communicator
        Ice::CommunicatorPtr itsComm;

        // Proxy object for remote service
        askap::interfaces::caldataservice::ICalibrationDataServicePrx itsService;

        // No support for assignment
        CalibrationDataServiceClient& operator=(const CalibrationDataServiceClient& rhs);

        // No support for copy constructor
        CalibrationDataServiceClient(const CalibrationDataServiceClient& src);
};

};
};
};

#endif
